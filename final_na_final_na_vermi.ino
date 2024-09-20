#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <Wire.h>

SoftwareSerial mySerial(14, 11); // RX, TX
int DE = 12;
int RE = 13;
const int relayPins[] = {35, 36, 37, 38};
const int moistureSensorPin = 15;

#define WIFI_SSID "catt0" 
#define WIFI_PASSWORD "SAlvADor2370146" 
#define API_KEY "AIzaSyB6eXL3DhYvRq7LRkZSfCSfb1UIz9dkm2M"
#define DATABASE_URL "https://verms-79d98-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define DHT_SENSOR_PIN 5
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

bool relayStatus = false;
bool lightStatus = true;
bool valveStatus = false;
bool pumpStatus = false;

void setup() {
  Serial.begin(115200);
  
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
  }
  
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

  dht_sensor.begin();
  
  mySerial.begin(4800);
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Authentication successful");
    signupOK = true;
  } else {
    Serial.printf("Authentication failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read all sensors
  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();
  int moistureLevel = analogRead(moistureSensorPin);
  float moisturePercentage = round((100 - ((moistureLevel / 4095.0) * 100)) * 10.0) / 10.0;
  unsigned int soilHumidity, soilTemperature, soilPH, nitrogen, potassium, phosphorus;
  readNPKSensor(soilHumidity, soilTemperature, soilPH, nitrogen, phosphorus, potassium);

  // Control relays based on sensor readings
  controlRelays(temperature, moisturePercentage, humidity);

  // Upload data to Firebase if conditions are met
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    uploadDataToFirebase(temperature, humidity, soilHumidity, soilTemperature, soilPH, nitrogen, phosphorus, potassium, moisturePercentage);
  }

  // Read Firebase states and control relays
  readFirebaseStatesAndControlRelays();


  delay(400);  // Minimal delay to avoid overloading
}

void readNPKSensor(unsigned int &soilHumidity, unsigned int &soilTemperature, unsigned int &soilPH, unsigned int &nitrogen, unsigned int &phosphorus, unsigned int &potassium) {
  byte queryData[]{0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];

  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  mySerial.write(queryData, sizeof(queryData));
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(1000);

  if (mySerial.available() >= sizeof(receivedData)) {
    mySerial.readBytes(receivedData, sizeof(receivedData));
    soilHumidity = (receivedData[3] << 8) | receivedData[4];
    soilTemperature = (receivedData[5] << 8) | receivedData[6];
    soilPH = (receivedData[9] << 8) | receivedData[10];
    nitrogen = (receivedData[11] << 8) | receivedData[12];
    phosphorus = (receivedData[13] << 8) | receivedData[14];
    potassium = (receivedData[15] << 8) | receivedData[16];
  }
}

void controlRelays(float temperature, float moisturePercentage, float humidity) {
  // Control fan relay
  relayStatus = temperature >= 28;
  digitalWrite(relayPins[0], relayStatus ? LOW : HIGH);

  // Control pump and valve relays
  if (moisturePercentage <= 50.0 || humidity <= 55.0) {
    turnOnPumpAndValve();
  } else {
    turnOffPumpAndValve();
  }
}

void uploadDataToFirebase(float temperature, float humidity, unsigned int soilHumidity, unsigned int soilTemperature, unsigned int soilPH, unsigned int nitrogen, unsigned int phosphorus, unsigned int potassium, float moisturePercentage) {
  Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature", temperature);
  Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity", humidity);
  Firebase.RTDB.setFloat(&fbdo, "NPK/SoilHumidity", soilHumidity / 10.0);
  Firebase.RTDB.setFloat(&fbdo, "NPK/SoilTemperature", soilTemperature / 10.0);
  Firebase.RTDB.setFloat(&fbdo, "NPK/SoilPH", soilPH / 10.0);
  Firebase.RTDB.setFloat(&fbdo, "NPK/Nitrogen", nitrogen);
  Firebase.RTDB.setFloat(&fbdo, "NPK/Phosphorus", phosphorus);
  Firebase.RTDB.setFloat(&fbdo, "NPK/Potassium", potassium);
  Firebase.RTDB.setFloat(&fbdo, "Chamber/Soil Moisture", moisturePercentage);
  Firebase.RTDB.setBool(&fbdo, "RelayStatus/Fan", relayStatus);
  Firebase.RTDB.setBool(&fbdo, "RelayStatus/Lights", lightStatus);
  Firebase.RTDB.setBool(&fbdo, "RelayStatus/Selenoid Valve", valveStatus);
  Firebase.RTDB.setBool(&fbdo, "RelayStatus/Waterpump", pumpStatus);
}

void readFirebaseStatesAndControlRelays() {
  bool conveyorStatus, separatorStatus, aeratorStatus;

  if (Firebase.RTDB.getBool(&fbdo, "Conveyor")) {
    conveyorStatus = fbdo.boolData();
    digitalWrite(relayPins[2], conveyorStatus ? LOW : HIGH);
  }

  if (Firebase.RTDB.getBool(&fbdo, "Separator")) {
    separatorStatus = fbdo.boolData();
    digitalWrite(relayPins[3], separatorStatus ? LOW : HIGH);
  }
}

void turnOnPumpAndValve() {
  digitalWrite(relayPins[1], LOW);
  pumpStatus = true;
  valveStatus = true;
}

void turnOffPumpAndValve() {
  digitalWrite(relayPins[1], HIGH);
  pumpStatus = false;
  valveStatus = false;
}
