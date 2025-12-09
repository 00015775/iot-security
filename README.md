# Components Logic
---

## Prerequisities

**Sensors:** 
- Ultrasonic distance sensor (HC-SR04) 
- Temperature & Humidity sensor (DHT11) 
- IR sensor 
- 4×4 Membrane Keypad 

**Actuators:** 
- Servo motor (SG90)
- LED 
- Buzzer (activate)
- Small Fan

**Sensor pins:**
- Ultrasonic (HC-SR04) → `TRIG`, `ECHO`
- DHT11 → digital pin
- IR sensor → digital pin
- Keypad → 8 pins (rows & columns)

**Atuator pins:**
- Servo → `PWM` pin
- LED → digital pin
- Buzzer → digital pin
- Fan → digital pin 

**Libraries:**
- `DHT.h` for DHT11
- `Keypad.h` for 4×4 keypad
- `Servo.h` for servo motor
- `NewPing` for Ultrasonic
- `ArduinoJson.h` for sending data

**Pin modes:**
- Sensors: `INPUT`
- Actuators: `OUTPUT`

**Default states:**
- Servo → 0° (door closed)
- LED → Off
- RGB → Off
- Buzzer → Off
- Fan → Off

**Communications:**
- USB serial cabel


## Main Loop

**Monitor the area using ultrasonic + IR sensors:**
- Ultrasonic distance sensor: Detects if someone is approaching.
    - If distance < “_approaching threshold_” → trigger voice/text alert:
          `"Someone is approaching the door"`
- IR sensor: Detects if someone is very close to the door.
    - If IR triggered OR ultrasonic distance < “_at-door threshold_” → trigger voice/text alert:
          `"Someone is at the door. Please enter the code on keypad"`

**Keypad input:**
- If correct code entered:
    - RGB LED → Green 
    - Servo → Rotate to “_door open_” (simulate door opening)
    - Voice/Text → `"Door is unlocked"`
    - Wait 3–4 seconds
    - Servo → Rotate back to 0° (door closes)
    - Turn RGB LED off

- If wrong code entered:
    - Buzzer → On for 1–2 seconds
    - RGB LED → Red for 1–2 seconds
    - Then turn Buzzer and RGB LED off

_No limit on number of attempts; repeat until correct code_

**Post-door-close action:**
- Monitor temperature using DHT11:
    - If temperature > threshold → Turn fan ON
    - Else → Fan OFF

**Loop continues: Monitor sensors → Door logic → Fan logic**

### Main Loop flow-diagram
![flow-diagram](./diagrams/uml-diagrams/flow-diagram.png)

## Circuit Diagram

![iot-security-diagram](./diagrams/circuit-diagrams/iot-security-full.png)

<details>
<summary>
<strong>To see where each wire and pin connects, click here:</strong>
</summary>

### Components were connected in the order provided below.

**Ultrasonic Sensor (HC-SR04):**
  - VCC → 5V
  - GND → GND
  - TRIG → Pin 12
  - ECHO → Pin 11

**DHT11 Temperature Sensor:**
  - VCC → 5V (in the middle if not defined)
  - GND → GND (or written as "-")
  - DATA → Pin 2 (or written as "S")

**IR Sensor:**
  - VCC → 5V
  - GND → GND
  - OUT → Pin 3

**4×4 Keypad:**
  - Row 1 → A0
  - Row 2 → A1
  - Row 3 → A2
  - Row 4 → A3
  - Column 1 → Pin 10
  - Column 2 → A4
  - Column 3 → A5
  - Column 4 → Pin 13

**Servo Motor (SG90):**
  - Red (VCC) → 5V
  - Brown (GND) → GND
  - Orange (Signal) → Pin 9

**RGB LED (Common VCC)**
  - Common Cathode → GND
  - Red pin → Pin 5 (with 220Ω resistor)
  - Blue pin → Pin 7 (with 220Ω resistor)

**Buzzer:**
  - Positive (+) → Pin 8
  - Negative (-) → GND

**Simple Fan (2-wire with NPN Transistor):**
Required Components:
  - 1x NPN Transistor (PN2222A)
  - 1x 1kΩ Resistor
  - 1x Diode 1N4007 (for protection)
<pre>
Arduino Pin 4 → 1kΩ Resistor → Transistor Base (middle pin)
Transistor Emitter (left/right pin) → Arduino GND
Transistor Collector (left/right pin) → Fan GND (-)
Fan VCC (+) → Arduino 5V
Diode (1N4007): Cathode to Arduino 5V, Anode to Fan GND
</pre>

</details>

## Network Diagram

![network-diagram](./diagrams/network-diagrams/network-diagram.png)

---
# Web Interface
---
## System Backlog 

### The web interface allows a user to:
  - Monitor all sensors in real time (_door status, temperature sensor, IR sensor_)
  - Control actuators (_door, fan, buzzer, LED_)
  - Manage system settings (_change keypad password, set ultrasonic/temperature threshold, reset settings_)
  - View sensor history, alerts, and statistics.


**Log Alerts:**
  - **Door Access:** Time of unlock, method (web or keypad).       
  - **Failed Password Attempts:** Keypad wrong entry, number of attempts.
  - **Motion Alerts:** "_Someone approaching_", "_Someone at the door_".
  - **Temperature Alerts:** “Fan ON due to high temp”.
  - **Features:** Filter by date/event type and export logs to CSV/JSON.
<pre>
| Timestamp | Event           | Source  | Extra      |
| --------- | --------------- | ------- | ---------- |
| 14:33     | Door unlocked   | Keypad  | Code: 1234 |
| 09:50     | Wrong attempt   | Keypad  | 3 tries    |
| 08:11     | Someone at door | Sensors | IR Trigger |
</pre>

**Control of actuators:**
  - **Servo (Door Lock):** Open / Close
  - **Fan:** Turn ON/OFF override
  - **RGB LED:** Choose (Red, Blue)
  - **Buzzer:** Turn ON/OFF (for alarm test)

**Password Management:**
  - Change the door unlock password (Arduino keypad code).
  - Require confirmation before saving.
  - Minimum 4 digits.
  - Must push new password to Arduino.

**Data Visualization:**
  - Temperature trend (line chart, last 30 seconds).
  - Number of door unlock attempts (bar chart).

**Data Analysis:**
  - Average Temperature: 23.5°C 
  - Total Visitors Today: 12
  - Failed Attempts Today: 3
  - Maximum Temperature Today: 29°C
  - Number of times Fan Activated: 5

**Device Settings:**
  - Set temperature threshold for fan.
  - Set ultrasonic thresholds (_someone approaching the door_).
  - Set a new door unlock password.
  - Reset settings to the default state.


