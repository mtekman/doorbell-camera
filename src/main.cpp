#include "esp_camera.h"
#include <WiFi.h>

// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM -- ESP32S3
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM -- ESP32
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD

#include "camera_pins.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

extern TaskHandle_t the_camera_loop_task;
extern void the_camera_loop (void* pvParameter);
esp_err_t init_sdcard();

void startCameraServer();

const char* def_ssid = "VPS11826";
const char* def_pass = "123456789";

#include <SD_MMC.h>

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
    }
}


void start_wifi() {
  String junk;
  String cssid;
  String cssid2;
  String cpass;
  char ssidch[32];
  char ssidch2[32];
  char passch[64];

  File config_file = SD_MMC.open("/secret.txt", "r");
  if (config_file) {

    Serial.println("Reading secret.txt");
    cssid = config_file.readStringUntil(' ');
    cssid2 = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    cpass = config_file.readStringUntil(' ');
    junk = config_file.readStringUntil('\n');
    config_file.close();

    cssid.toCharArray(ssidch, cssid.length() + 1);
    cssid2.toCharArray(ssidch2, cssid2.length() + 1);
    cpass.toCharArray(passch, cpass.length() + 1);

    if (String(cssid) == "ap" || String(cssid) == "AP") {
      WiFi.softAP(ssidch2, passch);

      IPAddress IP = WiFi.softAPIP();
      Serial.println("Using AP mode: ");
      Serial.print(ssidch2); Serial.print(" / "); Serial.println(passch);
      Serial.print("AP IP address: ");
      Serial.println(IP);
    } else {
      Serial.println(ssidch);
      Serial.println(passch);
      WiFi.begin(ssidch, passch);
      WiFi.setSleep(false);

      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      Serial.println("Using Station mode: ");
      Serial.print(cssid); Serial.print(" / "); Serial.println(cpass);
      Serial.println("WiFi connected");

      Serial.print("Camera Ready! Use 'http://");
      Serial.print(WiFi.localIP());
      Serial.println("' to connect");
    }
  } else {
    Serial.println("Failed to open config.txt - writing a default");

    // lets make a simple.txt config file
    File new_simple = SD_MMC.open("/secret.txt", "w");
    new_simple.println("ap ESP32-CAM // your ssid - ap mean access point mode, put Router123 station mode");
    new_simple.println("123456789    // your ssid password");
    new_simple.close();

    WiFi.softAP(def_ssid, def_pass);

    IPAddress IP = WiFi.softAPIP();
    Serial.println("Using AP mode: ");
    Serial.print(def_ssid); Serial.print(" / "); Serial.println(def_pass);
    Serial.print("AP IP address: ");
    Serial.println(IP);

  }
}

void led_blink(uint num_times = 2,
               uint delay1 = 50, uint delay2 = 50) {
  while (num_times-- > 0){
    delay(delay1);
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(delay2);
    digitalWrite(BLUE_LED_PIN, LOW);
  }
}



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
  esp_sleep_enable_timer_wakeup(2 * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(5) + " Seconds");


  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

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
    esp_sleep_enable_timer_wakeup(time_to_sleep * uS_TO_S_FACTOR);
    Serial.flush();
    //esp_deep_sleep_start(); // Boots to setup upon awakening
    led_blink(10);
    esp_light_sleep_start(); // Boots to loop upon awakening
    led_blink(3);
    loopCount = 0;
  }
}
