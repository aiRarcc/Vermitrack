#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <SoftwareSerial.h> // Include SoftwareSerial library for NPK sensor communication
#include <Wire.h>



SoftwareSerial mySerial(14, 11); // RX, TX 12 = 19 9 = 16 , 9 =11
int DE = 12; // 10 12
int RE = 13; //11
const int relayPins[] = {35, 36, 37, 38};
const int moistureSensorPin = 15;   

#define WIFI_SSID "catt0" // Oreo
#define WIFI_PASSWORD "SAlvADor2370146" //10987654321
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



void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
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
 
  if (temperature >= 28) {
    digitalWrite(relayPins[0], LOW);
    relayStatus = true;
  } else if (temperature <=27) {
    digitalWrite(relayPins[0], HIGH);
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
    /*Serial.print("Soil Humidity: ");
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
    Serial.println(potassium); */
    
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();
      // Upload DH11 data to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "DHT_11/Temperature", temperature)) {
        //Serial.print("Temperature : ");
        //Serial.println(temperature);
      } else {
        //Serial.println("Failed to upload temperature data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "DHT_11/Humidity", humidity)) {
        //Serial.print("Humidity : ");
        //Serial.println(humidity);
      } else {
        //Serial.println("Failed to upload humidity data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      // Upload NPK sensor data to Firebase
      if (Firebase.RTDB.setFloat(&fbdo, "NPK/SoilHumidity", soilHumidity / 10.0)) {
       // Serial.print("Soil Humidity: ");
      //  Serial.println((float)soilHumidity / 10.0);
      } else {
       // Serial.println("Failed to upload soil humidity data");
       // Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/SoilTemperature", soilTemperature / 10.0)) {
       // Serial.print("Soil Temperature: ");
        //Serial.println((float)soilTemperature / 10.0);
      } else {
       // Serial.println("Failed to upload soil temperature data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/SoilPH", soilPH / 10.0)) {
        //Serial.print("SoilPH: ");
        //Serial.println((float)soilPH / 10.0);
      } else {
        //Serial.println("Failed to upload soilPH data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Nitrogen", nitrogen)) {
        //Serial.print("Nitrogen: ");
        //Serial.println((float)soilTemperature);
      } else {
        //Serial.println("Failed to upload nitrogen data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Phosphorus", phosphorus)) {
        //Serial.print("Phosphorus: ");
        //Serial.println((float)phosphorus);
      } else {
        //Serial.println("Failed to upload phosphorus data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setFloat(&fbdo, "NPK/Potassium", potassium)) {
        //Serial.print("Potassium: ");
        //Serial.println((float)potassium);
      } else {
        //Serial.println("Failed to upload potassium data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Fan", relayStatus)) {
        //Serial.print("Fan: ");
        //Serial.println((bool)relayStatus);
      } else {
        //Serial.println("Failed to upload fanStatus data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }
      // Upload Chamber data to Firebase
      if (Firebase.RTDB.setFloat(&fbdo, "Chamber/Soil Moisture", moistpct)) {
        //Serial.print("Soil Moisture: ");
        //Serial.println((float)moistpct);
      } else {
        //Serial.println("Failed to upload moistpct data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Lights", lightStatus)) {
        //Serial.print("L: ");
        //Serial.println((bool)lightStatus);
      } else {
        //Serial.println("Failed to upload lightStatus data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Selenoid Valve", valveStatus)) {
        //Serial.print("V: ");
        //Serial.println((bool)valveStatus);
      } else {
        //Serial.println("Failed to upload valveStatus data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.setBool(&fbdo, "RelayStatus/Waterpump", pumpStatus)) {
        //Serial.print("W: ");
        //Serial.println((bool)pumpStatus);
      } else {
        //Serial.println("Failed to upload soil pumpStatus data");
        //Serial.println("REASON: " + fbdo.errorReason());
      }

      
    }
    
   // Delay for a short time to avoid conflicts with Firebase upload
    
    
  } 

  bool grinderStatus;
  bool separatorStatus;
  bool conveyorStatus;
  bool aeratorStatus;

  // Check if Firebase is ready
  if (Firebase.ready()) {
  // Read grinder data from Firebase
      if (Firebase.RTDB.getBool(&fbdo, "Conveyor")) {
        // Get the value of the grinder from Firebase
        conveyorStatus = fbdo.boolData();

        // Print the grinder status
        //Serial.print("Conveyor Status: ");
        //Serial.println(conveyorStatus);
      } else {
        //Serial.println("Failed to read grinder data");
        //Serial.println("Reason: " + fbdo.errorReason());
      }

        if (Firebase.RTDB.getBool(&fbdo, "Separator")) {
        // Get the value of the grinder from Firebase
        separatorStatus = fbdo.boolData();

        // Print the grinder status
        //Serial.print("Separator Status: ");
        //Serial.println(separatorStatus);
      } else {
        //Serial.println("Failed to read separator data");
        //Serial.println("Reason: " + fbdo.errorReason());
      }



        if (Firebase.RTDB.getBool(&fbdo, "Aerator")) {
        // Get the value of the grinder from Firebase
        aeratorStatus = fbdo.boolData();

        // Print the grinder status
        //Serial.print("Aerator Status: ");
        //Serial.println(aeratorStatus);
      } else {
        //Serial.println("Failed to read grinder data");
        //Serial.println("Reason: " + fbdo.errorReason());
      }
    } else {
      //Serial.println("Firebase is not ready");
    }

    if(conveyorStatus == true){
      digitalWrite(relayPins[2], LOW);
    }else{
      digitalWrite(relayPins[2], HIGH);
    }

    if(separatorStatus == true){
      digitalWrite(relayPins[3], LOW);
    }else{
      digitalWrite(relayPins[3], HIGH);
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
    bool con = conveyorStatus;
    bool sep = separatorStatus;
    bool aer = aeratorStatus;

    // Create a formatted string
    String data = String(soil, 1) + "," + String(tem, 1) + "," + String(hum, 1) + "," +
                  String(sH, 1) + "," + String(sT, 1) + "," + String(sPH, 1) + "," +
                  String(nit) + "," + String(phos) + "," + String(pot) + "," +
                  String(fan) + "," + String(light) + "," + String(pump) + "," +
                  String(val) + "," + String(con) + "," + String(sep) + "," + String(aer);

    // Send the string via serial communication
    Serial.println(data);


delay(650);
}

// Function to turn on both the pump and the solenoid valve
void turnOnPumpAndValve() {
  digitalWrite(relayPins[1], LOW);
  pumpStatus = true;
  valveStatus = true;
}

// Function to turn off both the pump and the solenoid valve
void turnOffPumpAndValve() {
  digitalWrite(relayPins[1], HIGH);
  pumpStatus = false;
  valveStatus = false;
}



