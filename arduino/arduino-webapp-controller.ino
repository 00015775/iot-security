/*
 * Smart Door System with USB Serial Communication
 * Direct USB connection to computer for web interface
 * 
 * Protocol: JSON commands over Serial (9600 baud)
 * 
 * References:
 * - Servo: https://lastminuteengineers.com/servo-motor-arduino-tutorial/
 * - DHT11: https://lastminuteengineers.com/dht11-module-arduino-tutorial/
 * - IR: https://lastminuteengineers.com/pir-sensor-arduino-tutorial/
 * - Keypad: https://lastminuteengineers.com/arduino-keypad-tutorial/
 * - Ultrasonic: https://lastminuteengineers.com/arduino-sr04-ultrasonic-sensor-tutorial/
 * - ArduinoJson: https://arduinojson.org/
 * - Code adapted from https://www.electronicsforu.com/electronics-projects/sensor-data-sending-over-web-serial
 * - Code adapted from https://forum.arduino.cc/t/servo-motor-with-a-ultrasonic-sensor/921109
 * - Code adapted from https://www.scribd.com/document/829854684/CODE-FOR-THE-PROJECT
 * - Code adapted from https://yedianyang.medium.com/how-to-send-data-to-a-web-page-from-arduino-serial-port-7aef849ccdd6
 
 */

#include <DHT.h>
#include <Keypad.h>
#include <Servo.h>
#include <NewPing.h>
#include <ArduinoJson.h>

// ===== PIN DEFINITIONS =====
#define TRIG_PIN 12
#define ECHO_PIN 11
#define MAX_DISTANCE 200

#define DHT_PIN 2
#define DHT_TYPE DHT11

#define IR_PIN 3
#define SERVO_PIN 9

#define RED_PIN 5
#define BLUE_PIN 7

#define BUZZER_PIN 8
#define FAN_PIN 4

// ===== THRESHOLDS, can be changed via web =====
int approachingDistance = 50;
int atDoorDistance = 15;
int tempThreshold = 10;
#define DOOR_OPEN_TIME 4000

// ===== KEYPAD SETUP =====
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {10, A4, A5, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ===== OBJECTS =====
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
DHT dht(DHT_PIN, DHT_TYPE);
Servo doorServo;

// ===== VARIABLES =====
String correctCode = "1234";  
String enteredCode = "";
bool doorOpen = false;
bool approachAlertShown = false;
bool doorAlertShown = false;
bool manualFanControl = false;
bool manualDoorControl = false;

unsigned long lastSensorSend = 0;
const long sensorInterval = 1000;

int wrongAttempts = 0;

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  
  pinMode(IR_PIN, INPUT);
  dht.begin();
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  setRGBColor(0, 0);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  
  sendLog("system_start", "arduino", "System initialized");
  delay(1000);
}

// ===== MAIN LOOP =====
void loop() {
  // Handle commands from web interface
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
  
  // Send sensor data periodically
  if (millis() - lastSensorSend >= sensorInterval) {
    lastSensorSend = millis();
    sendSensorData();
  }
  
  // Main system logic
  if (!manualDoorControl) {
    handleProximityDetection();
    handleKeypadInput();
  }
  
  if (!manualFanControl) {
    handleTemperatureControl();
  }
  
  delay(100);
}

// ===== SEND SENSOR DATA =====
void sendSensorData() {
  StaticJsonDocument<256> doc;
  
  int distance = sonar.ping_cm();
  if (distance == 0) distance = MAX_DISTANCE + 1;
  
  int irState = digitalRead(IR_PIN);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  doc["type"] = "sensor";
  doc["distance"] = distance;
  doc["ir"] = (irState == LOW);
  doc["temp"] = isnan(temperature) ? 0 : temperature;
  doc["humidity"] = isnan(humidity) ? 0 : humidity;
  doc["door"] = doorOpen;
  doc["fan"] = digitalRead(FAN_PIN);
  
  serializeJson(doc, Serial);
  Serial.println();
}

// ===== PROCESS WEB COMMANDS =====
void processCommand(String command) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, command);
  
  if (error) return;
  
  String cmd = doc["cmd"].as<String>();
  
  if (cmd == "open_door") {
    manualDoorControl = true;
    unlockDoor();
    sendLog("door_unlock", "web", "Manual open");
    manualDoorControl = false;
    
  } else if (cmd == "close_door") {
    closeDoor();
    sendLog("door_close", "web", "Manual close");
    
  } else if (cmd == "fan_on") {
    manualFanControl = true;
    digitalWrite(FAN_PIN, HIGH);
    sendLog("fan_on", "web", "Manual override");
    
  } else if (cmd == "fan_off") {
    manualFanControl = true;
    digitalWrite(FAN_PIN, LOW);
    sendLog("fan_off", "web", "Manual override");
    
  } else if (cmd == "fan_auto") {
    manualFanControl = false;
    sendLog("fan_auto", "web", "Auto mode enabled");
    
  } else if (cmd == "led_red") {
    setRGBColor(255, 0);
    
  } else if (cmd == "led_blue") {
    setRGBColor(0, 255);
    
  } else if (cmd == "led_off") {
    setRGBColor(0, 0);
    
  } else if (cmd == "buzzer") {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    
  } else if (cmd == "set_password") {
    String newPass = doc["value"].as<String>();
    if (newPass.length() >= 4) {
      correctCode = newPass;
      sendLog("password_change", "web", "Password updated");
    }
    
  } else if (cmd == "set_temp") {
    tempThreshold = doc["value"];
    sendLog("setting_change", "web", "Temp threshold: " + String(tempThreshold));
    
  } else if (cmd == "set_approach") {
    approachingDistance = doc["value"];
    sendLog("setting_change", "web", "Approach dist: " + String(approachingDistance));
    
  } else if (cmd == "set_door_dist") {
    atDoorDistance = doc["value"];
    sendLog("setting_change", "web", "Door dist: " + String(atDoorDistance));
    
  } else if (cmd == "reset") {
    correctCode = "1234";
    approachingDistance = 50;
    atDoorDistance = 15;
    tempThreshold = 10;
    sendLog("reset", "web", "Settings reset to defaults");
    
  } else if (cmd == "get_settings") {
    sendSettings();
  }
}

// ===== PROXIMITY DETECTION =====
void handleProximityDetection() {
  int distance = sonar.ping_cm();
  if (distance == 0) distance = MAX_DISTANCE + 1;
  
  int irState = digitalRead(IR_PIN);
  
  if (distance < approachingDistance && distance > atDoorDistance) {
    if (!approachAlertShown && !doorOpen) {
      sendLog("motion", "sensors", "Someone approaching");
      approachAlertShown = true;
      doorAlertShown = false;
    }
  }
  
  if (distance < atDoorDistance || irState == LOW) {
    if (!doorAlertShown && !doorOpen) {
      sendLog("at_door", "sensors", "Someone is at the door!");
      doorAlertShown = true;
      approachAlertShown = false;
    }
  }
  
  if (distance > approachingDistance) {
    approachAlertShown = false;
    doorAlertShown = false;
  }
}

// ===== KEYPAD INPUT =====
void handleKeypadInput() {
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {
      if (enteredCode == correctCode) {
        unlockDoor();
        sendLog("door_unlock", "keypad", "Code: " + enteredCode);
        wrongAttempts = 0;
      } else {
        wrongCodeAlert();
        wrongAttempts++;
        sendLog("wrong_code", "keypad", "Attempts: " + String(wrongAttempts));
      }
      enteredCode = "";
    }
    else if (key == '*') {
      enteredCode = "";
    }
    else {
      enteredCode += key;
    }
  }
}

// ===== TEMPERATURE CONTROL =====
void handleTemperatureControl() {
  float temperature = dht.readTemperature();
  
  if (!isnan(temperature)) {
    bool fanWasOn = digitalRead(FAN_PIN);
    
    if (temperature > tempThreshold) {
      if (!fanWasOn) {
        digitalWrite(FAN_PIN, HIGH);
        sendLog("fan_on", "auto", "Temp: " + String(temperature) + "C");
      }
    } else {
      if (fanWasOn) {
        digitalWrite(FAN_PIN, LOW);
        sendLog("fan_off", "auto", "Temp normal");
      }
    }
  }
}

// ===== DOOR CONTROL =====
void unlockDoor() {
  doorOpen = true;
  setRGBColor(255, 0);
  
  doorServo.attach(SERVO_PIN);
  delay(50);
  doorServo.write(90);
  
  delay(DOOR_OPEN_TIME);
  
  doorServo.write(0);
  delay(500);
  doorServo.detach();
  
  //setRGBColor(0, 0);
  doorOpen = false;
  doorAlertShown = false;
}

void closeDoor() {
  doorServo.attach(SERVO_PIN);
  delay(50);
  doorServo.write(0);
  delay(500);
  doorServo.detach();
  doorOpen = false;
}

void wrongCodeAlert() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1500);
  digitalWrite(BUZZER_PIN, LOW);
  setRGBColor(0, 0);
}

// ===== RGB LED =====
void setRGBColor(int red, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(BLUE_PIN, blue);
}

// ===== LOGGING =====
void sendLog(String event, String source, String extra) {
  StaticJsonDocument<256> doc;
  doc["type"] = "log";
  doc["event"] = event;
  doc["source"] = source;
  doc["extra"] = extra;
  doc["time"] = millis();
  
  serializeJson(doc, Serial);
  Serial.println();
}

// ===== SEND CURRENT SETTINGS =====
void sendSettings() {
  StaticJsonDocument<256> doc;
  doc["type"] = "settings";
  doc["temp_threshold"] = tempThreshold;
  doc["approach_dist"] = approachingDistance;
  doc["door_dist"] = atDoorDistance;
  doc["password"] = "****";
  
  serializeJson(doc, Serial);
  Serial.println();
}


