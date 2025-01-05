
#ifndef SLEEP_FUNCS_H
#define SLEEP_FUNCS_H

#define MICROSECONDS 1000000  /* Conversion factor for micro seconds to seconds */

#define LIGHT_SLEEP_AFTER_NOACTIVITY 20  // light sleep after N seconds no activity
#define LIGHT_SLEEP_WAKEUP_AFTER 20      // wake up after N seconds in light sleep
#define DEEP_SLEEP_AFTER_NOACTIVITY 40   // deep sleep after N seconds no activity

static esp_sleep_wakeup_cause_t inline print_wakeup_reason() {
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
  return wakeup_reason;
}

static void inline esp_wakeup_seconds(int seconds) {
  esp_sleep_enable_timer_wakeup(seconds * MICROSECONDS);
}

static void inline sleep_deep() {
  ESP_LOGI("Sleep", "going to DEEP sleep after %d seconds of no activity", DEEP_SLEEP_AFTER_NOACTIVITY);
  esp_deep_sleep_start();
}

static void inline sleep_light() {
  ESP_LOGI("Sleep", "going to LIGHT sleep for %ds after %ds of no activity at %d",
           LIGHT_SLEEP_WAKEUP_AFTER, LIGHT_SLEEP_AFTER_NOACTIVITY, millis());
  esp_sleep_enable_timer_wakeup(LIGHT_SLEEP_WAKEUP_AFTER * MICROSECONDS);
  esp_light_sleep_start();
  ESP_LOGI("Sleep", "Woke up at %d", millis());
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER); // Remove the timer wakeup
}

#endif // SLEEP_FUNCS_H