🐔 PoultryPortal — Automated Coop Door Controller
https://github.com/bbroper1/The_Poultry_Portal/actions/workflows/build.yml/badge.svg
https://img.shields.io/badge/license-MIT-blue.svg
https://img.shields.io/badge/platformio-ESP32-orange
https://img.shields.io/badge/version-v2.0.0-green
[Looks like the result wasn't safe to show. Let's switch things up and try something else!]
A fully modular, production‑grade ESP32 firmware for managing an automated poultry coop door with sunrise/sunset scheduling, battery monitoring, OTA updates, Telegram control, and a clean OLED UI.

🚀 Features
🌅 Smart Sunrise/Sunset Automation
- Automatically opens and closes based on real solar times
- Custom offsets for early/late open/close
- Boot‑time correction ensures the door always starts in the correct state
📡 Telegram Bot Control
- Open/close commands
- Full status reports
- Health checks
- Debug tools
- Secure user ID filtering
🔋 Battery + Power Monitoring
- INA219 voltage/current sensing
- Daily mAh tracking
- Low‑battery alerts
- Critical battery protection
🖥️ OLED Display (SSD1306)
- Live status
- Temperature
- Battery voltage
- Door state
- Two‑screen UI (Main + Info)
- Clean text‑only layout for reliability
🌡️ Temperature Monitoring
- Overheat protection
- Live temperature display
- Health reporting
🔧 Modular Architecture
Every subsystem is isolated into its own module:
- Motor
- Display
- Battery
- Energy
- Scheduler
- Telegram
- Time utilities
- WiFi setup
- Auto‑mode logic
- System status
- Logging
- Configuration
This makes the firmware easy to maintain, extend, and debug.
🔄 OTA Updates
- Secure OTA via ArduinoOTA
- Optional password protection
🛠️ Hardware Watchdog Safe
- Non‑blocking loop
- Regular WDT resets

📁 Project Structure
PoultryPortal/
└── src
    ├── main.cpp
    └── modules
        ├── automode
        │   ├── AutoModeTask.cpp
        │   └── AutoModeTask.h
        │
        ├── battery
        │   ├── BatterModule.cpp
        │   └── BatteryModule.h
        │
        ├── config
        │   ├── Config.cpp
        │   └── Config.h
        │
        ├── display
        │   ├── DisplayTask.cpp
        │   └── DisplayTask.h
        │
        ├── door
        │   ├── DoorController.cpp
        │   ├── DoorController.h
        │   ├── DoorModule.cpp
        │   └── DoorModule.h
        │
        ├── energy
        │   ├── EnergyModule.cpp
        │   └── EnergyModule.h
        │
        ├── hardware
        │   ├── HardwarePins.cpp
        │   └── HardwarePins.h
        │
        ├── keyboards
        │   ├── TelegramKeyboards.cpp
        │   └── TelegramKeyboards.h
        │
        ├── motor
        │   ├── MotorModule.cpp
        │   ├── MotorModule.h
        │   ├── MotorPins.cpp
        │   ├── MotorPins.h
        │   ├── MotorTask.cpp
        │   └── MotorTask.h
        │
        ├── power
        │   ├── PowerContext.cpp
        │   └── PowerContext.h
        │
        ├── scheduler
        │   ├── SchedulerTask.cpp
        │   ├── SchedulerTask.h
        │   ├── SunContext.cpp
        │   └── SunContext.h
        │
        ├── sensors
        │   ├── SensorModule.cpp
        │   ├── SensorModule.h
        │   ├── SensorTask.cpp
        │   └── SensorTask.h
        │
        ├── status
        │   ├── StatusMessage.cpp
        │   └── StatusMessage.h
        │
        ├── system
        │   ├── Globals.cpp
        │   ├── Globals.h
        │   ├── Logging.cpp
        │   ├── Logging.h
        │   ├── SupervisorTask.cpp
        │   ├── SupervisorTask.h
        │   ├── SystemStatus.cpp
        │   ├── SystemStatus.h
        │   ├── WiFiSetup.cpp
        │   └── WiFiSetup.h
        │
        ├── telegram
        │   ├── TelegramCertificate.h
        │   ├── TelegramClient.cpp
        │   ├── TelegramClient.h
        │   ├── TelegramEvent.h
        │   ├── TelegramHandler.cpp
        │   ├── TelegramHandler.h
        │   ├── TelegramKeyboards.h
        │   ├── TelegramMessage.h
        │   ├── TelegramTask.cpp
        │   ├── TelegramTask.h
        │   └── handlers
        │       ├── ConfigHandler.cpp
        │       ├── ConfigHandler.h
        │       ├── DebugHandler.cpp
        │       ├── DebugHandler.h
        │       ├── EnergyHandler.cpp
        │       ├── EnergyHandler.h
        │       ├── HealthHandler.cpp
        │       ├── HealthHandler.h
        │       ├── LogsHandler.cpp
        │       ├── LogsHandler.h
        │       ├── MenuHandler.cpp
        │       ├── MenuHandler.h
        │       ├── MotorHandler.cpp
        │       ├── MotorHandler.h
        │       ├── OffsetHandler.cpp
        │       ├── OffsetHandler.h
        │       ├── SettingHandler.cpp
        │       ├── SettingsHandler.h
        │       ├── SimHandler.cpp
        │       ├── SimHandler.h
        │       ├── StatusHandler.cpp
        │       ├── StatusHandler.h
        │       ├── TimezoneHandler.cpp
        │       └── TimezoneHandler.h
        │
        ├── time
        │   ├── TimeManager.cpp
        │   └── TimeManager.h
        │
        └── utils
            ├── MessageBus.h
            ├── StringUtils.cpp
            ├── StringUtils.h
            ├── TimeUtils.h
            └── TimeUtils_Compat.cpp

platformio.ini
Brent, I’ve got you — here’s a clean, polished, professionally formatted README you can copy‑paste directly.
I kept your structure, badges, and content, but tightened spacing, fixed formatting, and made the file tree readable and consistent.

🐔 PoultryPortal — Automated Coop Door Controller
https://github.com/bbroper1/The_Poultry_Portal/actions/workflows/build.yml/badge.svg
https://img.shields.io/badge/license-MIT-blue.svg
https://img.shields.io/badge/platformio-ESP32-orange
https://img.shields.io/badge/version-v2.0.0-green
[Looks like the result wasn't safe to show. Let's switch things up and try something else!]
A fully modular, production‑grade ESP32 firmware for managing an automated poultry coop door with sunrise/sunset scheduling, battery monitoring, OTA updates, Telegram control, and a clean OLED UI.

🚀 Features
🌅 Smart Sunrise/Sunset Automation
- Automatically opens and closes based on real solar times
- Custom offsets for early/late open/close
- Boot‑time correction ensures the door always starts in the correct state
📡 Telegram Bot Control
- Open/close commands
- Full status reports
- Health checks
- Debug tools
- Secure user ID filtering
🔋 Battery + Power Monitoring
- INA219 voltage/current sensing
- Daily mAh tracking
- Low‑battery alerts
- Critical battery protection
🖥️ OLED Display (SSD1306)
- Live status
- Temperature
- Battery voltage
- Door state
- Two‑screen UI (Main + Info)
- Clean text‑only layout for reliability
🌡️ Temperature Monitoring
- Overheat protection
- Live temperature display
- Health reporting
🔧 Modular Architecture
Every subsystem is isolated into its own module:
- Motor
- Display
- Battery
- Energy
- Scheduler
- Telegram
- Time utilities
- WiFi setup
- Auto‑mode logic
- System status
- Logging
- Configuration
This makes the firmware easy to maintain, extend, and debug.
🔄 OTA Updates
- Secure OTA via ArduinoOTA
- Optional password protection
🛠️ Hardware Watchdog Safe
- Non‑blocking loop
- Regular WDT resets

📁 Project Structure
PoultryPortal/
└── src
    ├── main.cpp
    └── modules
        ├── automode
        │   ├── AutoModeTask.cpp
        │   └── AutoModeTask.h
        │
        ├── battery
        │   ├── BatterModule.cpp
        │   └── BatteryModule.h
        │
        ├── config
        │   ├── Config.cpp
        │   └── Config.h
        │
        ├── display
        │   ├── DisplayTask.cpp
        │   └── DisplayTask.h
        │
        ├── door
        │   ├── DoorController.cpp
        │   ├── DoorController.h
        │   ├── DoorModule.cpp
        │   └── DoorModule.h
        │
        ├── energy
        │   ├── EnergyModule.cpp
        │   └── EnergyModule.h
        │
        ├── hardware
        │   ├── HardwarePins.cpp
        │   └── HardwarePins.h
        │
        ├── keyboards
        │   ├── TelegramKeyboards.cpp
        │   └── TelegramKeyboards.h
        │
        ├── motor
        │   ├── MotorModule.cpp
        │   ├── MotorModule.h
        │   ├── MotorPins.cpp
        │   ├── MotorPins.h
        │   ├── MotorTask.cpp
        │   └── MotorTask.h
        │
        ├── power
        │   ├── PowerContext.cpp
        │   └── PowerContext.h
        │
        ├── scheduler
        │   ├── SchedulerTask.cpp
        │   ├── SchedulerTask.h
        │   ├── SunContext.cpp
        │   └── SunContext.h
        │
        ├── sensors
        │   ├── SensorModule.cpp
        │   ├── SensorModule.h
        │   ├── SensorTask.cpp
        │   └── SensorTask.h
        │
        ├── status
        │   ├── StatusMessage.cpp
        │   └── StatusMessage.h
        │
        ├── system
        │   ├── Globals.cpp
        │   ├── Globals.h
        │   ├── Logging.cpp
        │   ├── Logging.h
        │   ├── SupervisorTask.cpp
        │   ├── SupervisorTask.h
        │   ├── SystemStatus.cpp
        │   ├── SystemStatus.h
        │   ├── WiFiSetup.cpp
        │   └── WiFiSetup.h
        │
        ├── telegram
        │   ├── TelegramCertificate.h
        │   ├── TelegramClient.cpp
        │   ├── TelegramClient.h
        │   ├── TelegramEvent.h
        │   ├── TelegramHandler.cpp
        │   ├── TelegramHandler.h
        │   ├── TelegramKeyboards.h
        │   ├── TelegramMessage.h
        │   ├── TelegramTask.cpp
        │   ├── TelegramTask.h
        │   └── handlers
        │       ├── ConfigHandler.cpp
        │       ├── ConfigHandler.h
        │       ├── DebugHandler.cpp
        │       ├── DebugHandler.h
        │       ├── EnergyHandler.cpp
        │       ├── EnergyHandler.h
        │       ├── HealthHandler.cpp
        │       ├── HealthHandler.h
        │       ├── LogsHandler.cpp
        │       ├── LogsHandler.h
        │       ├── MenuHandler.cpp
        │       ├── MenuHandler.h
        │       ├── MotorHandler.cpp
        │       ├── MotorHandler.h
        │       ├── OffsetHandler.cpp
        │       ├── OffsetHandler.h
        │       ├── SettingHandler.cpp
        │       ├── SettingsHandler.h
        │       ├── SimHandler.cpp
        │       ├── SimHandler.h
        │       ├── StatusHandler.cpp
        │       ├── StatusHandler.h
        │       ├── TimezoneHandler.cpp
        │       └── TimezoneHandler.h
        │
        ├── time
        │   ├── TimeManager.cpp
        │   └── TimeManager.h
        │
        └── utils
            ├── MessageBus.h
            ├── StringUtils.cpp
            ├── StringUtils.h
            ├── TimeUtils.h
            └── TimeUtils_Compat.cpp

platformio.ini



🔌 Hardware Requirements
- ESP32 Dev Module
- INA219 current/voltage sensor
- SSD1306 OLED (I²C)
- Limit switches (open/close detection)
- Motor driver (L298N, BTS7960, or similar)
- 12V battery or solar system
- Temperature sensor (DS18B20 or analog)

⚙️ Setup Instructions
1. Clone the repo
git clone https://github.com/bbroper1/The_Poultry_Portal.git
cd The_Poultry_Portal


2. Install PlatformIO
https://platformio.org/install
3. Configure WiFi + Telegram
Configure via Telegram bot or edit defaults in Config.cpp.
4. Build & Upload
pio run --target upload

5. OTA Updates
Once running:
pio run --target upload --upload-port <device-ip>

📲 Telegram Commands
|  |  | 
| /open |  | 
| /close |  | 
| /status |  | 
| /health |  | 
| /auto |  | 
| /offsets |  | 
| /debug |  | 

🧠 Architecture Overview
The firmware is built around clean separation of concerns:
- main.cpp → High‑level orchestration
- AutoMode → Sunrise/sunset logic
- Scheduler → Solar calculations
- Motor → Door movement + safety
- Battery → Voltage/current + alerts
- Energy → Daily mAh tracking
- Display → OLED UI
- TelegramRouter → Command routing
- TimeUtils → NTP + timezone + uptime
- WiFiSetup → WiFiManager wrapper
- SystemStatus → Health snapshots
- Logging → Persistent logs
- Config → Preferences storage
This structure makes the system robust, testable, and easy to extend.

🐛 Debugging
Enable debug mode via Telegram:
/debugon