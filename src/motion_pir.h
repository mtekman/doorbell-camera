
#ifndef MOTION_PIR_H
#define MOTION_PIR_H

#define MOTION_PIR_PIN GPIO_NUM_21 // GPIO pin for motion pullup
#define MOTION_PROLONG 5           // motion is prolonged N seconds after initial trigger
#define SLEEP_AFTER_NOACTIVITY 20  // deep sleep after N seconds no activity

boolean startTimer = false;
unsigned long lastTrigger = 0;

// Timer: Auxiliary variables
unsigned long now = millis();
boolean motion = false;
unsigned short noMotionCount = 0;

// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  digitalWrite(BLUE_LED_PIN, HIGH);
  startTimer = true;
  lastTrigger = millis();
  ESP_LOGI("Motion", " Started when %d", lastTrigger);
  motion = true;
  noMotionCount = 0;
}

inline void configure_pir_setup (){
  esp_sleep_wakeup_cause_t wakeup_reason = print_wakeup_reason();   //Print the wakeup reason for ESP32
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    detectsMovement(); // Assume woken up by movement
  } else {
    digitalWrite(BLUE_LED_PIN, LOW);
  }  
  //led_blink(3);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIR_PIN), detectsMovement, RISING);
  esp_sleep_enable_ext0_wakeup(MOTION_PIR_PIN, HIGH);
}

inline void motion_stop_after_prolong() {
  if (startTimer && (now - lastTrigger > (MOTION_PROLONG*1000))) {
    ESP_LOGI("Motion", " Stopped when %d", now);
    digitalWrite(BLUE_LED_PIN, LOW);
    startTimer = false;
    motion = false;
  }
}



#endif // MOTION_PIR_H