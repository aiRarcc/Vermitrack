#include "DHT.h"
#define DHTPIN D1
#define DHTTYPE DHT11
//#include <>

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

DHT dht (DHTPIN, DHTTYPE);

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Smart_Bro_25DF0"
#define WIFI_PASSWORD "Chanyeol@27"

#define API_KEY "AIzaSyA5gut-oJ7IGlaJbUhQIE1loOfAuIXM2R0"
#define DATABASE_URL"https://plantcare-v01-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

void setup(){
  pinMode(DHTPIN, INPUT);
  dht.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() !=WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
  Serial.println("ok");
  signupOK = true;
}
else{
  Serial.printf("%s\n", config.signer.signupError.message.c_str());
}

config.token_status_callback = tokenStatusCallback;

Firebase.begin(&config, &auth);
Firebase.reconnectWiFi (true);
}

void loop(){
  delay(1000);
  float h = dht.readHumidity();

  float t = dht.readTemperature();

  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.setFloat (&fbdo, "DHT/humidity", h)) {
      Serial.println("passed");
      Serial.print("Humidity: ");
      Serial.println(h);
    }
    else {
      Serial.println("failed");
      Serial.println("reason: " + fbdo.errorReason());
    }

    /*if (Firebase.RTDB.setFloat (&fbdo, "DHT/humidity", h)) {
      Serial.println("passed");
      Serial.print("Humidity: ");
      Serial.println(h);
    }
    else {
      Serial.println("failed");
      Serial.println("reason: " + fbdo.errorReason());
    }*/
  }
  Serial.println("__________________________");
}

