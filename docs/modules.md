# PoultryPortal Firmware — Module Documentation

This document provides an overview of each major module in the PoultryPortal firmware.  
The goal is to keep the codebase maintainable, modular, and easy to extend.

---

## 🖥 Display Module (`Display.cpp` / `Display.h`)

### Purpose
Handles all user‑facing visual output on the OLED display.

### Responsibilities
- Initialize the OLED (I²C)
- Render status screens (open/closed, battery, errors)
- Show boot messages
- Display configuration values
- Provide helper functions for drawing text and icons

### Interactions
- Reads system state from `Config` and `Motor`
- Called by `main.cpp` during loop updates

---

## ⚙️ Motor Module (`Motor.cpp` / `Motor.h`)

### Purpose
Controls the door motor through the DRV8871 driver.

### Responsibilities
- Open and close the door
- Drive IN1/IN2 pins for direction control
- Monitor limit switches
- Provide safety stop logic
- Integrate with INA219 for current‑based pinch detection
- Expose high‑level functions:
  - `openDoor()`
  - `closeDoor()`
  - `stopMotor()`
  - `isDoorOpen()`
  - `isDoorClosed()`

### Interactions
- Reads limit switches (GPIO 32/33)
- Reads current sensor via `INA219`
- Updates `Config` with door state
- Called by `main.cpp` and `Scheduler`

---

## ⚡ INA219 Module (`CurrentSensor.cpp` / `CurrentSensor.h`)

### Purpose
Provides current and voltage readings for safety and diagnostics.

### Responsibilities
- Initialize INA219 (I²C)
- Read bus voltage, shunt voltage, current, and power
- Provide pinch detection thresholds
- Expose helper functions:
  - `getCurrent()`
  - `getVoltage()`
  - `isPinchDetected()`

### Interactions
- Used by `Motor` for safety
- Shares I²C bus with `Display`

---

## ⚙️ Config Module (`Config.cpp` / `Config.h`)

### Purpose
Stores and manages persistent configuration values.

### Responsibilities
- Load and save settings (SPIFFS/LittleFS)
- Store:
  - open/close times
  - WiFi credentials
  - Telegram bot token/chat ID
  - door state
  - calibration values
- Provide getters and setters

### Interactions
- Used by `main.cpp`, `Motor`, `Display`, and `Telegram`

---

## 📡 Telegram Module (`Telegram.cpp` / `Telegram.h`)

### Purpose
Handles remote control and notifications via Telegram Bot API.

### Responsibilities
- Initialize Telegram bot
- Poll for incoming messages
- Parse commands:
  - `/open`
  - `/close`
  - `/status`
  - `/config`
- Send notifications:
  - Door opened/closed
  - Pinch detected
  - Errors or warnings

### Interactions
- Calls `Motor` to open/close door
- Reads `Config` for settings
- Sends status messages using system state

---

## 🌐 WiFi Module (`WiFiManager.cpp` / `WiFiManager.h`)

### Purpose
Manages WiFi connectivity.

### Responsibilities
- Connect to stored SSID/password
- Retry logic
- Provide connection status
- Expose helper functions:
  - `connectWiFi()`
  - `isConnected()`

### Interactions
- Used by `Telegram`
- Used by OTA (if implemented)

---

## 🕒 Scheduler Module (`Scheduler.cpp` / `Scheduler.h`)

### Purpose
Automates door open/close based on time.

### Responsibilities
- Compare current time to configured schedule
- Trigger `Motor` actions
- Handle sunrise/sunset logic (if implemented)
- Provide next‑event calculations

### Interactions
- Reads times from `Config`
- Calls `Motor` to open/close door

---

## 🧠 Main Application (`main.cpp`)

### Purpose
Entry point of the firmware.

### Responsibilities
- Initialize all modules
- Run main loop
- Update display
- Poll Telegram
- Run scheduler
- Handle state transitions

### Interactions
- Coordinates all modules
- Acts as the glue layer

---

# 📁 Directory Structure (for reference)

/src
├── main.cpp
├── Display.cpp
├── Motor.cpp
├── Config.cpp
├── Telegram.cpp
├── Scheduler.cpp
├── WiFiManager.cpp
├── CurrentSensor.cpp
/include
├── Display.h
├── Motor.h
├── Config.h
├── Telegram.h
├── Scheduler.h
├── WiFiManager.h
├── CurrentSensor.h
/docs
├── wiring.md
├── modules.md  
