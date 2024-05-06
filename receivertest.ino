#include <espnow.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display


// Structure for temperature and humidity data
typedef struct VermiTrackData {
  float temperature;
  float humidity;
  float soilmoisture;
  int nitrogen;
  int phosphorus;
  int potassium;
  float soilph;
  float soilhum;
  float soiltemp;
  bool fanrelay;
  bool lightrelay;
  bool valverelay;
  bool pumprelay;
} VermiTrackData;

// Create a TemperatureHumidityData called sensorData
VermiTrackData sensorData;


// callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&sensorData, incomingData, sizeof(sensorData));
  delay(100);
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Temperature: ");
  Serial.println(sensorData.temperature);
  Serial.print("Humidity: ");
  Serial.println(sensorData.humidity);
  Serial.print("SoilMoisture: ");
  Serial.println(sensorData.soilmoisture);
  Serial.print("Nitrogen: ");
  Serial.println(sensorData.nitrogen);
  Serial.print("Phosphorus: ");
  Serial.println(sensorData.phosphorus);
  Serial.print("Potassium: ");
  Serial.println(sensorData.potassium);
  Serial.print("Soil PH: ");
  Serial.println(sensorData.soilph);
  Serial.print("Soil Temp: ");
  Serial.println(sensorData.soiltemp);
  Serial.print("Soil Hum: ");
  Serial.println(sensorData.soilhum);
  Serial.print("Fan Relay: ");
  Serial.println(sensorData.fanrelay);
  Serial.print("Light Relay: ");
  Serial.println(sensorData.lightrelay);
  Serial.print("Valve Relay: ");
  Serial.println(sensorData.valverelay);
  Serial.print("Pump Relay: ");
  Serial.println(sensorData.pumprelay);
  Serial.println();
  
  

}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);


  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) { // Note the return value difference for ESP8266
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE); // Set the ESP8266 as a slave
  esp_now_register_recv_cb(OnDataRecv);

}

void loop() {
  
}
