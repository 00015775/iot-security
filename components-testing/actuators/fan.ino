int fanPin = 9;  // Arduino pin controlling transistor

void setup() {
  pinMode(fanPin, OUTPUT);
}

void loop() {
  // Turn fan ON
  digitalWrite(fanPin, HIGH);
  delay(2000); // fan runs for 2 seconds

  // Turn fan OFF
  digitalWrite(fanPin, LOW);
  delay(2000); // fan off for 2 seconds
}


