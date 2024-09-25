#include "esp_camera.h"
#include <WiFi.h>
#include <SD_MMC.h>

// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

extern TaskHandle_t the_camera_loop_task;
extern void the_camera_loop (void* pvParameter);
esp_err_t init_sdcard();

void startCameraServer();

#include "sleep_funcs.h"
#include "pin_config.h"
#include "led_init.h"
#include "wifi_init.h"

RTC_DATA_ATTR int bootCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("--------------");
  Serial.println("Camera Server\n");
  Serial.println("--------------");

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */
  esp_wakeup_seconds(2);
  Serial.println("Setup ESP32 to sleep for every " + String(5) + " Seconds");


  camera_config_t config;
  initialize_pins(config);

  pinMode(BLUE_LED_PIN, OUTPUT); // Initialize the LED pin as an output
  led_blink(3);

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 3;      //jz 2->3 - add another frame for the avi recording
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  esp_err_t  card_err = init_sdcard();
  if (card_err != ESP_OK) {
    Serial.printf("SD Card init failed with error 0x%x", card_err);
  }

  /* Serial.println("Going to light sleep now"); */
  /* delay(1000); */
  /* Serial.flush();  */
  /* esp_light_sleep_start(); */

  start_wifi();

  /* Serial.println("Started Wifi. Going to light sleep now"); */
  /* delay(1000); */
  /* Serial.flush(); */
  /* esp_light_sleep_start(); // resumes here after awakening */

  startCameraServer();

  Serial.println("Started Camera Server");

  // 5000 stack, prio 5 same at http streamer, core 1
  xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 5000, NULL, 5, &the_camera_loop_task, 1);

  delay(100);
}


RTC_DATA_ATTR int noDetectCount = 0; // preserves accross reboots
int sleepMult = 1;
int loopCount = 0;

void loop() {
  Serial.print("\r");
  Serial.print("Loop: " + String(loopCount));
  delay(500);
  if (++loopCount > 30){
    int time_to_sleep = ++noDetectCount * 10;
    Serial.println("\nNothing. Deep sleep for " + String(time_to_sleep) + " seconds for every 1 minute of inactivity");
    esp_wakeup_seconds(time_to_sleep * uS_TO_S_FACTOR);
    Serial.flush();
    //esp_deep_sleep_start(); // Boots to setup upon awakening
    led_blink(10);
    esp_light_sleep_start(); // Boots to loop upon awakening
    led_blink(3);
    loopCount = 0;
  }
}
