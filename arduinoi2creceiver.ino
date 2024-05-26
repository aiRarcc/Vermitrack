#include <Arduino.h>
#include "U8glib.h"

float soil;
float tem;
float hum;
float sH;
float sT;
float sPH;
int nit;
int phos;
int pot;
bool fan;
bool light;
bool pump;
bool val;
bool gri;
bool sep;
bool aer;

// Initialize the display (uncomment the correct one for your display)
//U8GLIB_NHD27OLED_BW u8g(13, 11, 10, 9); // SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9
U8GLIB_ST7920_128X64_4X u8g(10); // ST7920 128x64 display using hardware SPI on pin 10

void setup() {
    Serial.begin(115200); // Start serial communication

    // Set the contrast of the display
    u8g.setContrast(50); // Adjust the contrast value as needed

    delay(1000);
}

void loop() {
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n'); // Read incoming string until newline character
        parseData(data); // Parse the received data string
    }

    // Your code to handle the parsed data
    // For example, printing the received values
    Serial.print("Soil: "); Serial.println(soil);
    Serial.print("Temperature: "); Serial.println(tem);
    Serial.print("Humidity: "); Serial.println(hum);
    Serial.print("Soil Humidity: "); Serial.println(sH);
    Serial.print("Soil Temperature: "); Serial.println(sT);
    Serial.print("Soil pH: "); Serial.println(sPH);
    Serial.print("Nitrogen: "); Serial.println(nit);
    Serial.print("Phosphorus: "); Serial.println(phos);
    Serial.print("Potassium: "); Serial.println(pot);
    Serial.print("Fan Status: "); Serial.println(fan);
    Serial.print("Light Status: "); Serial.println(light);
    Serial.print("Pump Status: "); Serial.println(pump);
    Serial.print("Valve Status: "); Serial.println(val);
    Serial.print("Grinder Status: "); Serial.println(gri);
    Serial.print("Separator Status: "); Serial.println(sep);
    Serial.print("Aerator Status: "); Serial.println(aer);

    // Picture loop for u8glib
    u8g.firstPage();
    do {
    draw();
    } while (u8g.nextPage());

  // Add any additional code you want to run repeatedly

    delay(1000); // Add a delay to control the loop frequency
}

void draw() {
  // Set the font for the display
  u8g.setFont(u8g_font_5x8);
  u8g.setColorIndex(1); // Set color to black (1) for the text
  
  // Create strings from variables
  char buffer[20]; // Buffer to hold the strings

  // Displaying float variables
  snprintf(buffer, sizeof(buffer), "Soil: %.2f", soil);
  u8g.drawStr(0, 6, buffer);
  
  snprintf(buffer, sizeof(buffer), "Temp: %.2f", tem);
  u8g.drawStr(0, 14, buffer);
  
  snprintf(buffer, sizeof(buffer), "Hum: %.2f", hum);
  u8g.drawStr(0, 22, buffer);
  
  snprintf(buffer, sizeof(buffer), "sH: %.2f", sH);
  u8g.drawStr(0, 30, buffer);
  
  snprintf(buffer, sizeof(buffer), "sT: %.2f", sT);
  u8g.drawStr(0, 38, buffer);
  
  snprintf(buffer, sizeof(buffer), "sPH: %.2f", sPH);
  u8g.drawStr(0, 46, buffer);
  
  // Displaying integer variables
  snprintf(buffer, sizeof(buffer), "N: %d", nit);
  u8g.drawStr(0, 54, buffer);
  
  snprintf(buffer, sizeof(buffer), "P: %d", phos);
  u8g.drawStr(50, 6, buffer); // Adjust x and y coordinates as needed
  
  snprintf(buffer, sizeof(buffer), "K: %d", pot);
  u8g.drawStr(50, 14, buffer);

  // Displaying boolean variables
  snprintf(buffer, sizeof(buffer), "Fan: %s", fan ? "ON" : "OFF");
  u8g.drawStr(50, 22, buffer);
  
  snprintf(buffer, sizeof(buffer), "Light: %s", light ? "ON" : "OFF");
  u8g.drawStr(50, 30, buffer);
  
  snprintf(buffer, sizeof(buffer), "Pump: %s", pump ? "ON" : "OFF");
  u8g.drawStr(50, 38, buffer);
  
  snprintf(buffer, sizeof(buffer), "Valve: %s", val ? "ON" : "OFF");
  u8g.drawStr(50, 46, buffer);
  
  snprintf(buffer, sizeof(buffer), "Grinder: %s", gri ? "ON" : "OFF");
  u8g.drawStr(50, 54, buffer);
  
  snprintf(buffer, sizeof(buffer), "Separator: %s", sep ? "ON" : "OFF");
  u8g.drawStr(50, 62, buffer);
  
  snprintf(buffer, sizeof(buffer), "Aerator: %s", aer ? "ON" : "OFF");
  u8g.drawStr(100, 6, buffer); // Adjust x and y coordinates as needed
}


void parseData(String data) {
    int startIndex = 0;
    int endIndex = data.indexOf(',');

    soil = data.substring(startIndex, endIndex).toFloat();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    tem = data.substring(startIndex, endIndex).toFloat();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    hum = data.substring(startIndex, endIndex).toFloat();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    sH = data.substring(startIndex, endIndex).toFloat();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    sT = data.substring(startIndex, endIndex).toFloat();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    sPH = data.substring(startIndex, endIndex).toFloat();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    nit = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    phos = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    pot = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    fan = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    light = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    pump = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    val = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    gri = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    endIndex = data.indexOf(',', startIndex);
    sep = data.substring(startIndex, endIndex).toInt();

    startIndex = endIndex + 1;
    aer = data.substring(startIndex).toInt();
}
