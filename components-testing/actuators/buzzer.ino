int buzzerPin = 8;

void setup() {
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  digitalWrite(buzzerPin, HIGH); // turn buzzer ON
  delay(500);                    // wait 0.5 second
  digitalWrite(buzzerPin, LOW);  // turn buzzer OFF
  delay(500);                    // wait 0.5 second
}

