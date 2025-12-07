/*
 * Smart Door System with Security Features
 * Compatible with Arduino UNO R3
 * 
 * Features:
 * - Ultrasonic distance detection (HC-SR04)
 * - IR proximity detection
 * - 4x4 Keypad authentication
 * - DHT11 temperature monitoring
 * - Servo motor door control
 * - RGB LED status indication
 * - Buzzer alerts
 * - Fan temperature control (Simple 2-wire fan)
 */

// ===== LIBRARIES =====
#include <DHT.h>
#include <Keypad.h>
#include <Servo.h>
#include <NewPing.h>

// ===== PIN DEFINITIONS =====
// Ultrasonic Sensor (HC-SR04)
#define TRIG_PIN 12
#define ECHO_PIN 11
#define MAX_DISTANCE 200  // Maximum distance in cm

// DHT11 Temperature Sensor
#define DHT_PIN 2
#define DHT_TYPE DHT11

// IR Sensor
#define IR_PIN 3

// Servo Motor
#define SERVO_PIN 9

// RGB LED Pins
#define RED_PIN 5
#define BLUE_PIN 7

// Buzzer
#define BUZZER_PIN 8

// Fan (Simple 2-wire fan via transistor)
#define FAN_PIN 4

// ===== THRESHOLDS =====
#define APPROACHING_DISTANCE 50  // cm - someone approaching
#define AT_DOOR_DISTANCE 15      // cm - someone at door
#define TEMP_THRESHOLD 10        // °C - fan activation temperature
#define DOOR_OPEN_TIME 4000      // ms - how long door stays open

// ===== KEYPAD SETUP =====
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Connect keypad ROW pins to Arduino pins
byte rowPins[ROWS] = {A0, A1, A2, A3};
// Connect keypad COLUMN pins to Arduino pins
byte colPins[COLS] = {10, A4, A5, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ===== OBJECT INSTANCES =====
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
DHT dht(DHT_PIN, DHT_TYPE);
Servo doorServo;

// ===== GLOBAL VARIABLES =====
String correctCode = "1234";  
String enteredCode = "";
bool doorOpen = false;
bool approachAlertShown = false;
bool doorAlertShown = false;

// ===== SETUP FUNCTION =====
void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println("=================================");
  Serial.println("Smart Door System Starting...");
  Serial.println("=================================");
  
  // Initialize Sensors
  pinMode(IR_PIN, INPUT);
  dht.begin();
  
  // Initialize Actuators
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);  // Start with door closed
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  // Set default states
  setRGBColor(0, 0);  // RGB off
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  
  Serial.println("System Ready!");
  Serial.println("Default code: 1234");
  Serial.println("Press '#' to submit, '*' to clear");
  Serial.println("=================================\n");
  delay(1000);
}

// ===== MAIN LOOP =====
void loop() {
  // Read sensor values
  int distance = sonar.ping_cm();
  int irState = digitalRead(IR_PIN);
  float temperature = dht.readTemperature();
  
  // Handle distance of 0 (out of range)
  if (distance == 0) {
    distance = MAX_DISTANCE + 1;
  }
  
  // ===== PROXIMITY DETECTION =====
  
  // Check if someone is approaching
  if (distance < APPROACHING_DISTANCE && distance > AT_DOOR_DISTANCE) {
    if (!approachAlertShown && !doorOpen) {
      Serial.println(">>> Someone is approaching the door");
      approachAlertShown = true;
      doorAlertShown = false;
    }
  }
  
  // Check if someone is at the door
  if (distance < AT_DOOR_DISTANCE || irState == LOW) {
    if (!doorAlertShown && !doorOpen) {
      Serial.println(">>> Someone is at the door. Please enter the code on keypad");
      doorAlertShown = true;
      approachAlertShown = false;
    }
    
    // ===== KEYPAD INPUT HANDLING =====
    char key = keypad.getKey();
    
    if (key) {
      Serial.print("Key pressed: ");
      Serial.println(key);
      
      if (key == '#') {  // Submit code
        Serial.print("Code entered: ");
        Serial.println(enteredCode);
        
        if (enteredCode == correctCode) {
          // CORRECT CODE
          Serial.println(">>> Door is unlocked");
          unlockDoor();
        } else {
          // WRONG CODE
          Serial.println(">>> Wrong code! Try again.");
          wrongCodeAlert();
        }
        
        enteredCode = "";  // Reset entered code
      }
      else if (key == '*') {  // Clear code
        enteredCode = "";
        Serial.println("Code cleared");
      }
      else {
        // Add digit to code
        enteredCode += key;
      }
    }
  }
  
  // Reset alerts when person moves away
  if (distance > APPROACHING_DISTANCE) {
    approachAlertShown = false;
    doorAlertShown = false;
  }
  
  // ===== TEMPERATURE CONTROL =====
  if (!isnan(temperature)) {
    if (temperature > TEMP_THRESHOLD) {
      digitalWrite(FAN_PIN, HIGH);
      // Uncomment to see fan status
      Serial.print("Fan ON - Temp: ");
      Serial.println(temperature);
    } else {
      digitalWrite(FAN_PIN, LOW);
    }
  }
  
  delay(100);  // Small delay for stability
}

// ===== HELPER FUNCTIONS =====

// Function to unlock door (correct code)
void unlockDoor() {
  doorOpen = true;
  
  // Turn RGB LED Blue
  setRGBColor(255, 0);
  
  // Open door (servo to 90 degrees)
  doorServo.write(90);
  
  // Wait with door open
  delay(DOOR_OPEN_TIME);
  
  // Close door
  doorServo.write(0);
  
  // Turn off RGB LED
  setRGBColor(0, 0);
  
  doorOpen = false;
  doorAlertShown = false;
}

// Function for wrong code alert
void wrongCodeAlert() {

  // Activate buzzer
  digitalWrite(BUZZER_PIN, HIGH);
  
  // Wait 1.5 seconds
  delay(1500);
  
  // Turn off buzzer and RGB
  digitalWrite(BUZZER_PIN, LOW);
  setRGBColor(0, 0);
}

// Function to set RGB LED color
void setRGBColor(int red, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(BLUE_PIN, blue);
}


