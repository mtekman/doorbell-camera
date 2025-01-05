
#ifndef RECORDING_STUFF_H
#define RECORDING_STUFF_H

bool isRecording = false;

void startRecordingActivity(){
  if (digitalRead(BLUE_LED_PIN) == HIGH){
    ESP_LOGI("RECORDING", "started");
    isRecording = true;
    delay(1000);
  } else {
    ESP_LOGI("RECORDING", "finished");
    isRecording = false;
  }
}

void publishRecording (){
  ESP_LOGI("PUBLISH", "Publishing motion");
  //mqtt_client.setServer(MQTT_SERVER, 1883);
}

void recordTheMotionThenResetWifi(){
  startRecordingActivity();
  if (!isRecording){
    setup_wifi();
    publishRecording();
    stop_wifi(); // an active WIFI seems to affect the PIR sensor.... yeah.
  }
}


#endif // RECORDING_STUFF_H