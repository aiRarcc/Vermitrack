const int sensor_pin = 15; // Example analog pin on ESP32-S3//A0 pin to 15 pin//

void setup() {
  Serial.begin(9600); // Define baud rate for serial communication
}

void loop() {
  float moisture_percentage;
  int sensor_analog;
  sensor_analog = analogRead(sensor_pin);
  moisture_percentage = ( 100 - ( (sensor_analog/4095.00) * 100 ) ); // ESP32 ADC has a 12-bit resolution (4095)
  Serial.print("Moisture Percentage = ");
  Serial.print(moisture_percentage);
  Serial.print("%\n\n");
  delay(100);
}
