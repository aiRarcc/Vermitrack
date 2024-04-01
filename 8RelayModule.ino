#define RELAY_COUNT 8
#define RELAY_ON_TIME 500 // in milliseconds

// Define pins for the relay module
int relayPins[RELAY_COUNT] = {2, 3, 4, 5, 6, 7, 8, 9};

void setup() {
  // Initialize relay pins as outputs
  for (int i = 0; i < RELAY_COUNT; i++) {
    pinMode(relayPins[i], OUTPUT);
  }
}

void loop() {
  // Turn on relays from 1 to 8
  for (int i = 0; i < RELAY_COUNT; i++) {
    digitalWrite(relayPins[i], HIGH);
    delay(RELAY_ON_TIME);
    digitalWrite(relayPins[i], LOW);
  }
  
  // Turn on relays from 8 to 1
  for (int i = RELAY_COUNT - 1; i >= 0; i--) {
    digitalWrite(relayPins[i], HIGH);
    delay(RELAY_ON_TIME);
    digitalWrite(relayPins[i], LOW);
  }
}
