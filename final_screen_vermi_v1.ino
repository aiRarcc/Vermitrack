#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Arduino.h>

// ----------------------------
// Touch Screen pins and SPI setup
// ----------------------------
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();  // TFT display object

// Firebase setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define WIFI_SSID "Oreo" // Oreo
#define WIFI_PASSWORD "10987654321" //10987654321
#define API_KEY "AIzaSyB6eXL3DhYvRq7LRkZSfCSfb1UIz9dkm2M"
#define DATABASE_URL "https://verms-79d98-default-rtdb.firebaseio.com/"

bool signupOK = false;
int currentSection = 0;  // Track current section for display
unsigned long lastTouchTime = 0;
unsigned long debounceDelay = 300;  // 300 ms debounce delay for touch

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  
  // Initialize Firebase
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

  // Start the SPI for the touch screen and init the TS library
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);

  // Initialize TFT display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  displaySection();  // Display the first section
}

void loop() {
  if (ts.tirqTouched() && ts.touched() && millis() - lastTouchTime > debounceDelay) {
    TS_Point p = ts.getPoint();
    lastTouchTime = millis();  // Update last touch time for debounce
    currentSection = (currentSection + 1) % 4;  // Cycle through 4 sections
    displaySection();  // Update the display with the new section
  }
}

void displaySection() {
  tft.fillScreen(TFT_BLACK);  // Clear the screen before displaying new section
  int x = 320 / 2;  // Center x position for text
  int y = 40;
  int fontSize = 4;

  switch (currentSection) {
    case 0:
      tft.drawCentreString("Worm Bin", x, y, fontSize);
      displayWormBinData();
      break;
    case 1:
      tft.drawCentreString("Soil Content", x, y, fontSize);
      displaySoilContentData();
      break;
    case 2:
      tft.drawCentreString("Motors", x, y, fontSize);
      displayMotorsData();
      break;
    case 3:
      tft.drawCentreString("Compost", x, y, fontSize);
      displayCompostData();
      break;
  }
}

void displayWormBinData() {
  // Retrieve and display data for "Worm Bin" section from Firebase
  int y = 100;
  if (Firebase.RTDB.getFloat(&fbdo, "Chamber/Soil Moisture")) {
    tft.drawString("Soil Moisture: " + String(fbdo.floatData()) + " %", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getFloat(&fbdo, "DHT_11/Temperature")) {
    tft.drawString("Temperature: " + String(fbdo.floatData()) + " °C", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getFloat(&fbdo, "DHT_11/Humidity")) {
    tft.drawString("Humidity: " + String(fbdo.floatData()) + " %", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getBool(&fbdo, "RelayStatus/Fan")) {
    tft.drawString("Fan: " + String(fbdo.boolData() ? "In Operation" : "On Standby"), 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getBool(&fbdo, "RelayStatus/Lights")) {
    tft.drawString("Light: " + String(fbdo.boolData() ? "In Operation" : "On Standby"), 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getBool(&fbdo, "RelayStatus/Waterpump")) {
    tft.drawString("Sprinkler: " + String(fbdo.boolData() ? "In Operation" : "On Standby"), 10, y, 2);
    y += 20;
  }
}

void displaySoilContentData() {
  // Retrieve and display data for "Soil Content" section from Firebase
  int y = 100;
  if (Firebase.RTDB.getFloat(&fbdo, "NPK/SoilTemperature")) {
    tft.drawString("Soil Temperature: " + String(fbdo.floatData()) + " C", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getFloat(&fbdo, "NPK/SoilHumidity")) {
    tft.drawString("Soil Humidity: " + String(fbdo.floatData()) + "%", 10, y, 2);
    y += 20;
  }

  if (Firebase.RTDB.getFloat(&fbdo, "NPK/SoilPH")) {
    float phValue = fbdo.floatData();
    String phType;

    if (phValue >= 1 && phValue <= 6) {
        phType = "Acidic";
    } else if (phValue == 7) {
        phType = "Neutral";
    } else if (phValue >= 8 && phValue <= 14) {
        phType = "Alkaline";
    } else {
        phType = "Calibrating"; // In case the pH value is out of range
    }

    // Display the pH value and its classification
    tft.drawString("Soil pH: " + String(phValue) + " (" + phType + ")", 10, y, 2);
    y += 20; // Move down for the next line
  }

  if (Firebase.RTDB.getFloat(&fbdo, "NPK/Nitrogen")) {
    tft.drawString("Nitrogen: " + String(fbdo.floatData()) + " mg/kg", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getFloat(&fbdo, "NPK/Phosphorus")) {
    tft.drawString("Phosphorus: " + String(fbdo.floatData()) + " mg/kg", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getFloat(&fbdo, "NPK/Potassium")) {
    tft.drawString("Potassium: " + String(fbdo.floatData()) + " mg/kg", 10, y, 2);
    y += 20;
  }
}

void displayMotorsData() {
  // Retrieve and display data for "Motors" section from Firebase
  int y = 100;
  if (Firebase.RTDB.getBool(&fbdo, "Conveyor")) {
    tft.drawString("Conveyor: " + String(fbdo.boolData() ? "In Operation" : "On Standby"), 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getBool(&fbdo, "Separator")) {
    tft.drawString("Trommel: " + String(fbdo.boolData() ? "In Operation" : "On Standby"), 10, y, 2);
    y += 20;
  }
}

void displayCompostData() {
  // Retrieve and display data for "Compost" section from Firebase
  int y = 100;
  if (Firebase.RTDB.getFloat(&fbdo, "DHT_11/Temperature")) {
    tft.drawString("Temperature: " + String(fbdo.floatData()) + " °C", 10, y, 2);
    y += 20;
  }
  if (Firebase.RTDB.getFloat(&fbdo, "DHT_11/Humidity")) {
    tft.drawString("Humidity: " + String(fbdo.floatData()) + " %", 10, y, 2);
    y += 20;
  }
}
