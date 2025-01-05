#include "esp_camera.h"
#include <SD_MMC.h>

// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

#include "sleep_funcs.h"
#include "led_init.h"
#include "recording_stuff.h" // -- should be before motion_pir.h
#include "motion_pir.h"
//#include "mqtt_stuff.h" -- TODO

esp_err_t init_sdcard();

RTC_DATA_ATTR int bootCount = 0;
//RTC_DATA_ATTR int noDetectCount = 0; // preserves accross reboots


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("START: Boot number: " + String(++bootCount));
  
  pinMode(BLUE_LED_PIN, OUTPUT); // Initialize the LED pin as an output  
  pinMode(MOTION_PIR_PIN, INPUT_PULLUP);

  // The detectMovement function happens outside of all loops.
  configure_pir_setup();
}

void loop() {
  now = millis();
  //ESP_LOGI("Loop", " count %d and noMotioncount %d",
  ESP_LOGI("Loop", " noMotioncount %d", noMotionCount);

  motion_stop_after_prolong();

  if (motion == false){
    ++noMotionCount;
    if (noMotionCount >= DEEP_SLEEP_AFTER_NOACTIVITY){
      sleep_deep();
    }
    if (noMotionCount % LIGHT_SLEEP_AFTER_NOACTIVITY == 0){
      sleep_light();
    }
  }
  delay(1000);
}
