#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <SoftwareSerial.h> // Include SoftwareSerial library for NPK sensor communication

// Define pin configurations for NPK sensor
SoftwareSerial mySerial(12, 9); // RX, TX
int DE = 10;
int RE = 11;

const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;
String GAS_ID = "AKfycby7MkvWqhAb4Ghg71BykLWjrDW9O9HjGATJPAF42Q1Xo0QyhUb-jqu6CWEYP7nqFsV4qg";
//https://script.google.com/macros/s/AKfycby7MkvWqhAb4Ghg71BykLWjrDW9O9HjGATJPAF42Q1Xo0QyhUb-jqu6CWEYP7nqFsV4qg/exec
// Initialize Firebase variables and credentials
#define WIFI_SSID "Oreo" // Oreo
#define WIFI_PASSWORD "10987654321" //10987654321
#define API_KEY "AIzaSyAG6hMlwJUI4giNNMmaUCqVPB-Zy8rl5fk"
#define DATABASE_URL "https://finalverm-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Initialize DHT sensor
#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
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
  client.setInsecure();
  
}

void loop() {
  // Read temperature and humidity from DHT sensor
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
        Serial.println("Failed to upload soil temperature data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Nitrogen", nitrogen)) {
        Serial.print("Nitrogen: ");
        Serial.println((float)soilTemperature);
      } else {
        Serial.println("Failed to upload soil temperature data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Phosphorus", phosphorus)) {
        Serial.print("Phosphorus: ");
        Serial.println((float)phosphorus);
      } else {
        Serial.println("Failed to upload soil temperature data");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Potassium", potassium)) {
        Serial.print("Potassium: ");
        Serial.println((float)potassium);
      } else {
        Serial.println("Failed to upload soil temperature data");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      // Add more Firebase.RTDB.setInt calls for other sensor data
    }
  }
  sendData(temperature, humidity, soilHumidity, soilTemperature, soilPH, nitrogen, potassium, phosphorus);
  delay(300000); // Wait for 5 minute before sending data again
}

void sendData(float tem, float hum, float sT, float sH, float sPH, float nit, float phos, float pot) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  
  String string_temperature =  String(tem);
  String string_humidity =  String(hum); 
  String string_soiltemp =  String(sT / 10.0);
  String string_soilhum =  String(sH / 10.0); 
  String string_soilph =  String(sPH /10.0);
  String string_nitrogen =  String(nit); 
  String string_phosphorus =  String(phos);
  String string_potassium =  String(pot); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&soiltemp=" + string_soiltemp +
  "&soilhumid=" + string_soilhum + "&soilph=" + string_soilph + "&nitrogen=" + string_nitrogen + "&phosphorus=" + string_phosphorus + "&potassium=" + string_potassium ;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  
  unsigned long timeout = millis();
  while (client.connected()) {
    if (millis() - timeout > 10000) { // Timeout after 10 seconds
      Serial.println("connection timeout");
      client.stop();
      return;
    }
  }
  
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("Data uploaded successfully!");
  } else {
    Serial.println("Data upload failed");
  }
  
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
}
