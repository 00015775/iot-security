#include <DHT.h>

#define DHT_PIN 2
#define DHT_TYPE DHT11
#define FAN_PIN 4
#define TEMP_THRESHOLD 10  // Low for testing

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  Serial.println("\n--- DHT11 & Fan Test ---");
  
  dht.begin();
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  Serial.println("Stabilizing sensor...");
  delay(2000);

  // Quick fan test
  Serial.println("\n>> Testing fan directly...");
  digitalWrite(FAN_PIN, HIGH);
  Serial.println("Fan ON (5 seconds)");
  delay(5000);
  digitalWrite(FAN_PIN, LOW);
  Serial.println("Fan OFF (3 seconds)");
  delay(3000);
  Serial.println("Did the fan spin? Check wiring if not.\n");
}

void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("DHT11 ERROR: Check wiring!");
  } else {
    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print("°C  | Humidity: ");
    Serial.print(h);
    Serial.println("%");

    if (t > TEMP_THRESHOLD) {
      digitalWrite(FAN_PIN, HIGH);
      Serial.println("Fan ON (Temp above threshold)");
    } else {
      digitalWrite(FAN_PIN, LOW);
      Serial.println("Fan OFF (Temp below threshold)");
    }
  }

  Serial.println("-----------------------------");
  delay(2000);
}



