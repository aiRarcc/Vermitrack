#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <SoftwareSerial.h> // Include SoftwareSerial library for NPK sensor communication
#include <Wire.h>
#include <esp_now.h>


SoftwareSerial mySerial(14, 11); // RX, TX 12 = 19 9 = 16 , 9 =11
int DE = 12; // 10 12
int RE = 13; //11
const int relayPins[] = {35, 36, 37, 38, 39, 40, 41, 42};
const int moistureSensorPin = 15;   

#define WIFI_SSID "Oreo" // Oreo
#define WIFI_PASSWORD "10987654321" //10987654321
#define API_KEY "AIzaSyB6eXL3DhYvRq7LRkZSfCSfb1UIz9dkm2M"
#define DATABASE_URL "https://verms-79d98-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Initialize DHT sensor
#define DHT_SENSOR_PIN 5
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

bool relayStatus = false;
bool lightStatus = true;
bool valveStatus = false;
bool pumpStatus = false;

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xD8, 0xBF, 0xC0, 0xEE, 0xBF, 0x3A};

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

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
  }

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize DHT sensor
  dht_sensor.begin();

  // Initialize NPK sensor pins
  mySerial.begin(4800);
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  // Initialize Firebase config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Authentication successful");
    signupOK = true;
  } else {
    Serial.printf("Authentication failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();
  // Declare variables for other sensor readings
  unsigned int soilHumidity, soilTemperature, soilPH, nitrogen, potassium, phosphorus;
  byte queryData[]{0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  mySerial.write(queryData, sizeof(queryData));
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(1000);

  int moistureLevel = analogRead(moistureSensorPin);

  float moisturePercentage = (100- ((moistureLevel / 4095.00) * 100));
  float moistpct = round(moisturePercentage * 10.0) / 10.0;
  digitalWrite(relayPins[0], LOW);
  if (temperature >= 28) {
    digitalWrite(relayPins[1], LOW);
    relayStatus = true;
  } else if (temperature <=27) {
    digitalWrite(relayPins[1], HIGH);
    relayStatus = false;
  }

  if (moistpct <= 50.0 || humidity <= 55.0) {
    // If moisture level is less 55 percent, turn on both the pump and the solenoid valve
    turnOnPumpAndValve();
    // Print soil moisture and status
  } else {
    // If moisture level is greater than 55 percent, turn off both the pump and the solenoid valve
    turnOffPumpAndValve();
    // Print soil moisture and status
  }

  if (mySerial.available() >= sizeof(receivedData)) {
    mySerial.readBytes(receivedData, sizeof(receivedData));
    // Parse and print the received data in decimal format
    soilHumidity = (receivedData[3] << 8) | receivedData[4];
    soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
    soilPH = (receivedData[9] << 8) | receivedData[10];
    nitrogen = (receivedData[11] << 8) | receivedData[12];
    phosphorus = (receivedData[13] << 8) | receivedData[14];
    potassium = (receivedData[15] << 8) | receivedData[16];
    
    // Print the received data
    Serial.print("Soil Humidity: ");
    Serial.println((float)soilHumidity / 10.0);
    Serial.print("Soil Temperature: ");
    Serial.println((float)soilTemperature / 10.0);
    Serial.print("Soil Conductivity: ");
    Serial.println(soilConductivity);
    Serial.print("Soil pH: ");
    Serial.println((float)soilPH / 10.0);
    Serial.print("Nitrogen: ");
    Serial.println(nitrogen);
    Serial.print("Phosphorus: ");
    Serial.println(phosphorus);
    Serial.print("Potassium: ");
    Serial.println(potassium);
    
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      // Upload DH11 data to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature", temperature)) {
        Serial.print("Temperature : ");
        Serial.println(temperature);
      } else {
        Serial.println("Failed to upload temperature data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity", humidity)) {
        Serial.print("Humidity : ");
        Serial.println(humidity);
      } else {
        Serial.println("Failed to upload humidity data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      // Upload NPK sensor data to Firebase
      if (Firebase.RTDB.setFloat(&fbdo, "NPK/SoilHumidity", soilHumidity / 10.0)) {
        Serial.print("Soil Humidity: ");
        Serial.println((float)soilHumidity / 10.0);
      } else {
        Serial.println("Failed to upload soil humidity data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/SoilTemperature", soilTemperature / 10.0)) {
        Serial.print("Soil Temperature: ");
        Serial.println((float)soilTemperature / 10.0);
      } else {
        Serial.println("Failed to upload soil temperature data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/SoilPH", soilPH / 10.0)) {
        Serial.print("SoilPH: ");
        Serial.println((float)soilPH / 10.0);
      } else {
        Serial.println("Failed to upload soilPH data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Nitrogen", nitrogen)) {
        Serial.print("Nitrogen: ");
        Serial.println((float)soilTemperature);
      } else {
        Serial.println("Failed to upload nitrogen data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Phosphorus", phosphorus)) {
        Serial.print("Phosphorus: ");
        Serial.println((float)phosphorus);
      } else {
        Serial.println("Failed to upload phosphorus data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Potassium", potassium)) {
        Serial.print("Potassium: ");
        Serial.println((float)potassium);
      } else {
        Serial.println("Failed to upload potassium data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Fan", relayStatus)) {
        Serial.print("Fan: ");
        Serial.println((bool)relayStatus);
      } else {
        Serial.println("Failed to upload fanStatus data");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      // Upload Chamber data to Firebase
      if (Firebase.RTDB.setFloat(&fbdo, "Chamber/Soil Moisture", moistpct)) {
        Serial.print("Soil Moisture: ");
        Serial.println((float)moistpct);
      } else {
        Serial.println("Failed to upload moistpct data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Lights", lightStatus)) {
        Serial.print("L: ");
        Serial.println((bool)lightStatus);
      } else {
        Serial.println("Failed to upload lightStatus data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Selenoid Valve", valveStatus)) {
        Serial.print("V: ");
        Serial.println((bool)valveStatus);
      } else {
        Serial.println("Failed to upload valveStatus data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Waterpump", pumpStatus)) {
        Serial.print("W: ");
        Serial.println((bool)pumpStatus);
      } else {
        Serial.println("Failed to upload soil pumpStatus data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      
    }
    
   // Delay for a short time to avoid conflicts with Firebase upload
    
    
  }
    float soil = moistpct;
    float tem = temperature;
    float hum = humidity;
    float sH = soilHumidity / 10.0;
    float sT = soilTemperature / 10.0;
    float sPH = soilPH / 10.0;
    int nit = nitrogen;
    int phos = phosphorus;
    int pot = potassium;
    bool fan = relayStatus;
    bool light = lightStatus;
    bool pump = pumpStatus;
    bool val = valveStatus;

    sensorData.temperature = tem;
    sensorData.humidity = hum;
    sensorData.soilmoisture = soil;
    sensorData.soilhum = sH;
    sensorData.soiltemp = sT;
    sensorData.soilph = sPH;
    sensorData.nitrogen = nit;
    sensorData.phosphorus = phos;
    sensorData.potassium = pot;
    sensorData.fanrelay = fan;
    sensorData.lightrelay = light;
    sensorData.pumprelay = pump;
    sensorData.valverelay = val;


    /*soilHumidity = (receivedData[3] << 8) | receivedData[4];
    soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
    soilPH = (receivedData[9] << 8) | receivedData[10];
    nitrogen = (receivedData[11] << 8) | receivedData[12];
    phosphorus = (receivedData[13] << 8) | receivedData[14];
    potassium = (receivedData[15] << 8) | receivedData[16]; */

    /*bool relayStatus = false;
    bool lightStatus = true;
    bool valveStatus = false;
    bool pumpStatus = false;*/

    // Send temperature and humidity data via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sensorData, sizeof(sensorData));
   
    if (result == ESP_OK) {
    Serial.println(" data sent with success");
    }
    else {
    Serial.println("Error sending data");
    }

    delay(5000);
}

// Function to turn on both the pump and the solenoid valve
void turnOnPumpAndValve() {
  digitalWrite(relayPins[2], LOW);
  digitalWrite(relayPins[3], LOW);
  pumpStatus = true;
  valveStatus = true;
}

// Function to turn off both the pump and the solenoid valve
void turnOffPumpAndValve() {
  digitalWrite(relayPins[2], HIGH);
  digitalWrite(relayPins[3], HIGH);
  pumpStatus = false;
  valveStatus = false;
}


