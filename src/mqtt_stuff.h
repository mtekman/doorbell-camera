
#ifndef MQTT_STUFF_H
#define MQTT_STUFF_H

#define MQTT_SERVER "homeassistant.local"

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish("outTopic","hello world");
      // ... and resubscribe
      mqtt_client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendPayload (){
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  Send data to Home Assistant
  String payload = "Your sensor data here";
  mqtt_client.publish("home/sensor1", payload.c_str());
}

#endif // MQTT_STUFF_H