#include <LiquidCrystal_I2C.h>  // Include the LiquidCrystal_I2C library
LiquidCrystal_I2C lcd(0x27, 16, 2);




// Define the structure for sensor data
struct SensorData {
  float temperature;
  float humidity;
  float soilmoisture;
  float nitrogen;
  float phosphorus;
  float potassium;
  float soilph;
  float soiltemp;
  float soilhum;
  int fanrelay;
  int lightrelay;
  int valverelay;
  int pumprelay;
};

void setup() {
  // Initialize Serial communication
  lcd.init();
  lcd.backlight(); // Turn on the backlight
 
  // Initialize Serial1 for communication with NodeMCU
  Serial.begin(9600); 
}

void loop() {
  if (Serial.available() > 0) {
    // Read the length of the data packet
    int len = Serial.parseInt();
    
    // Read the sensor data
    SensorData sensorData;
    sensorData.temperature = Serial.parseFloat();
    sensorData.humidity = Serial.parseFloat();
    sensorData.soilmoisture = Serial.parseFloat();
    sensorData.nitrogen = Serial.parseFloat();
    sensorData.phosphorus = Serial.parseFloat();
    sensorData.potassium = Serial.parseFloat();
    sensorData.soilph = Serial.parseFloat();
    sensorData.soiltemp = Serial.parseFloat();
    sensorData.soilhum = Serial.parseFloat();
    sensorData.fanrelay = Serial.parseInt();
    sensorData.lightrelay = Serial.parseInt();
    sensorData.valverelay = Serial.parseInt();
    sensorData.pumprelay = Serial.parseInt();
    
    // Print the sensor data
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



    String lcd_temperature = String(sensorData.temperature, 1); // 1 decimal place
    String lcd_humidity = String(sensorData.humidity, 1); // 1 decimal place
    String lcd_soiltemp = String(sensorData.soiltemp, 1); // 1 decimal place
    String lcd_soilhum = String(sensorData.soilhum, 1); // 1 decimal place
    String lcd_soilph = String(sensorData.soilph, 1); // 1 decimal place
    String lcd_nitrogen = String(sensorData.nitrogen, 1); // 1 decimal place
    String lcd_phosphorus = String(sensorData.phosphorus, 1); // 1 decimal place
    String lcd_potassium = String(sensorData.potassium, 1); // 1 decimal place
    String lcd_mst= String(sensorData.soilmoisture, 1); // 1 decimal place
    String lcd_fan = String(sensorData.fanrelay);
    String lcd_light = String(sensorData.lightrelay);
    String lcd_pump = String(sensorData.pumprelay);
    String lcd_valve = String(sensorData.valverelay);

    if (lcd_temperature == " 0.0 " || lcd_humidity == " 0.0" || lcd_soiltemp == "0.0" ||
        lcd_soilhum == "0.0" || lcd_soilph == "0.0" || lcd_nitrogen == "0.0" ||
        lcd_potassium == "0.0" || lcd_phosphorus == "0.0" || lcd_mst == "0.0") {
      // If any value is zero, ignore the data
      return;
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   Chamber ");
    lcd.setCursor(0,1);
    lcd.print("Temp: ");
    lcd.print(lcd_temperature + "C");
    delay(5000);
    lcd.setCursor(0,1);
    lcd.print("Humid: ");
    lcd.print(lcd_humidity + "%");
    delay(5000);
  //
    lcd.setCursor(0,0);
    lcd.print("Soil Status ");
    lcd.setCursor(0,1);
    lcd.print("                "); // Clear the second line
    lcd.setCursor(0,1);
    lcd.print("Temp: ");
    lcd.print(lcd_soiltemp + " C");
    delay(5000);
    lcd.setCursor(0,1);
    lcd.print("Humid: ");
    lcd.print(lcd_soilhum + " %");
    delay(5000);
    lcd.setCursor(0,1);
    lcd.print("PH: ");
    lcd.print(lcd_soilph + "            ");
    delay(5000);
    lcd.setCursor(0,1);
    lcd.print("N: ");
    lcd.print(lcd_nitrogen + " mg/kg");
    delay(5000);
    lcd.setCursor(0,1);
    lcd.print("P: ");
    lcd.print(lcd_phosphorus + " mg/kg");
    delay(5000);
    lcd.setCursor(0,1);
    lcd.print("K: ");
    lcd.print(lcd_potassium+ " mg/kg");
    delay(5000);

    
  }
}
