
#ifndef LED_FUNCTIONS_H
#define LED_FUNCTIONS_H

static void inline led_blink(int num_times = 2, int delay1 = 50, int delay2 = 50) {
  while (num_times-- > 0){
    delay(delay1);
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(delay2);
    digitalWrite(BLUE_LED_PIN, LOW);
  }
}

#endif // LED_FUNCTIONS_H