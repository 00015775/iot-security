int irPin = 8;  // IR sensor output pin

void setup() {
  pinMode(irPin, INPUT);   // IR sensor as input
  Serial.begin(9600);      // Start serial communication
}

void loop() {
  int value = digitalRead(irPin);  // Read sensor value
  Serial.println(value);           // Print value (0 or 1)
  delay(100);                      // small delay
}

/*
Reference:
Code adapted from https://lastminuteengineers.com/pir-sensor-arduino-tutorial/
*/

