````markdown
# ESP8266 Soil‐Moisture & Valve Control — README

A simple ESP8266 sketch that connects to Firebase Realtime Database to:

1. **Listen** for `"ON"` / `"OFF"` commands under `/users/{uid}/…/irrigate` and actuate a relay (valve + LED).
2. **Periodically** (every 10 min) read an analog soil-moisture sensor and upload its value under `/users/{uid}/…/soil-moisture/{timestamp}`.

---

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Pin Wiring](#pin-wiring)
- [Firebase Setup](#firebase-setup)
- [Code Configuration](#code-configuration)
- [Flashing the ESP](#flashing-the-esp)
- [How It Works](#how-it-works)
- [Troubleshooting & Tips](#troubleshooting--tips)

---

## Hardware Requirements

- ESP8266-based board (e.g. NodeMCU, Wemos D1 Mini)
- Relay module (to switch your solenoid valve)
- Soil-moisture sensor (analog output)
- LEDs (optional status indicators)
- Jumper wires, breadboard or PCB

---

## Pin Wiring

| ESP8266 Pin    | Function              | Hookup                                 |
| -------------- | --------------------- | -------------------------------------- |
| `A0`           | Soil-moisture input   | → sensor analog output                 |
| `D5` (GPIO 14) | Relay control         | → relay IN (active LOW)                |
| `D1` (GPIO 5)  | LED “Wi-Fi connected” | → LED + resistor → GND                 |
| `D2` (GPIO 4)  | LED “Valve ON”        | → LED + resistor → GND                 |
| `D0` (GPIO 16) | LED “Error / Init”    | → LED + resistor → GND                 |
| `FLASH` button | Manual “send” trigger | Optional – triggers soil-moisture send |

> **Note:** The relay is driven **active LOW**, so writing `LOW` closes the relay.

---

## Firebase Setup

1. **Realtime Database**
   - Create a Firebase project and enable Realtime Database in **“Test mode”** or with appropriate [security rules].
2. **Credentials**
   - In your project settings, under **Service accounts → Database secrets**, copy the **database secret**.
3. **Populate the code constants**
   ```cpp
   const char* ssid       = "YOUR_WIFI_SSID";
   const char* password   = "YOUR_WIFI_PASS";
   const char* uid        = "<YOUR_FIREBASE_UID>";
   const char* farmId     = "<YOUR_FARM_ID>";
   const char* cropId     = "<YOUR_CROP_ID>";
   #define FIREBASE_HOST "your-project-id.firebaseio.com"
   #define FIREBASE_AUTH "<YOUR_DATABASE_SECRET>"
   ```
````

4. **Database structure** (example):

   ```
   /users/{uid}/
     farms/{farmId}/
       crops/{cropId}/
         irrigate      → "ON" or "OFF"
         soil-moisture/
           {timestamp}:  450
   ```

---

## Code Configuration

In the top section of `sketch.ino`, adjust:

```cpp
// Wi-Fi network
const char* ssid     = "SSID";
const char* password = "WIFI_PASSWORD";

// Identifiers in your RTDB tree
const char* uid      = "dYo3S…";
const char* farmId   = "a6MrKs…";
const char* cropId   = "1eea98…";

// Firebase
#define FIREBASE_HOST "iot-farminc-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "rIAZZb…"
```

- **INTERVAL** controls how often (ms) you log soil moisture. Default is 10 minutes.
- LEDs array `[16,5,4]` reflect:

  - Index 2 (GPIO 4): Wi-Fi status
  - Index 1 (GPIO 5): Firebase connected
  - Index 0 (GPIO 16): Valve ON

---

## Flashing the ESP

1. Install the [ESP8266 board package] in Arduino IDE.
2. Select **“NodeMCU 1.0 (ESP-12E Module)”** and correct COM port.
3. Copy the sketch, adjust constants, and click **Upload**.
4. Open **Serial Monitor** at **9600 baud** to watch startup logs.

---

## How It Works

1. **Startup Sequence**

   - LEDs all OFF → LED\[2] (red) ON while connecting to Wi-Fi.
   - On successful connection → LED\[2] OFF, LED\[1] (green) ON.
   - Firebase initialized; test‐write to `/test/message`.

2. **Main Loop**

   - **Every second**:

     - Read `/…/irrigate`; execute `"ON"` (close relay) or `"OFF"` (open relay), update LED\[0].

   - **Every 10 minutes**:

     - Read analog soil moisture → upload under `…/soil-moisture/{millis()}`.

---

## Troubleshooting & Tips

- **“Firebase get failed”**

  - Check `FIREBASE_HOST` / `.json` suffix and network connectivity.

- **No Wi-Fi LED change**

  - Verify SSID/password and router range.

- **Soil-moisture values off**

  - Calibrate the sensor by reading raw `analogRead(A0)` over a wet–dry range.

- **Security**

  - For production, lock down your RTDB rules so only authenticated users read/write their own nodes.

---

<sub>© 2025 FarmInc IoT • MIT License</sub>

```markdown

```

[security rules]: https://firebase.google.com/docs/rules
[ESP8266 board package]: https://arduino.esp8266.com/stable/package_esp8266com_index.json
