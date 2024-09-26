#ifndef WIFI_INIT_H
#define WIFI_INIT_H

const char* def_ssid = "VPS11826";
const char* def_pass = "123456789";

void onWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("Client connected");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("Client disconnected");
      break;
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

#endif // WIFI_INIT_H