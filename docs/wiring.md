🪛 PoultryPortal Wiring Diagram (DRV8871 + INA219 + OLED + Limit Switches)
🧠 ESP32 → Peripherals Overview
Everything connects back to the ESP32, which acts as the central controller:

Code
ESP32
├── OLED Display (I²C)
├── INA219 Current Sensor (I²C)
├── DRV8871 Motor Driver
└── Limit Switches (Open & Closed)
All grounds must be tied together.

🖥 OLED Display Wiring (I²C)
Code
OLED VCC  → 3.3V
OLED GND  → GND
OLED SDA  → GPIO 21
OLED SCL  → GPIO 22
The OLED and INA219 share the same SDA/SCL lines.

🔌 INA219 Current Sensor Wiring (I²C + Motor Power Path)
Logic connections:
Code
INA219 VCC  → 3.3V
INA219 GND  → GND
INA219 SDA  → GPIO 21  (shared with OLED)
INA219 SCL  → GPIO 22  (shared with OLED)
Power measurement path:
Code
12V Supply (+) → INA219 VIN+ 
INA219 VIN−    → DRV8871 VM (motor power input)
This lets the INA219 measure:

motor current

motor supply voltage

power consumption

Perfect for pinch detection and battery monitoring.

⚙️ DRV8871 Motor Driver Wiring
Logic pins:
Code
DRV8871 IN1 → GPIO 25
DRV8871 IN2 → GPIO 26
Power:
Code
DRV8871 VM  → 12V (from INA219 VIN−)
DRV8871 GND → GND (shared with ESP32)
Motor terminals:
Code
DRV8871 OUT1 → Motor lead 1
DRV8871 OUT2 → Motor lead 2
Direction is controlled by IN1/IN2.

🚪 Limit Switch Wiring (Active‑Low)
Door Open Switch:
Code
Switch COM  → GND
Switch NO   → GPIO 32
Door Closed Switch:
Code
Switch COM  → GND
Switch NO   → GPIO 33
ESP32 internal pull‑ups keep the pins HIGH until the switch is pressed.

🔋 Power Wiring
Code
ESP32 VIN/5V  → 5V supply or USB
ESP32 GND     → GND

12V (+)       → INA219 VIN+
INA219 VIN−   → DRV8871 VM
DRV8871 GND   → GND
ESP32 GND     → GND
OLED GND      → GND
INA219 GND    → GND
All grounds must be common or the system will behave unpredictably.

🧩 Full Wiring Diagram (ASCII Block Layout)
Code
                   ┌──────────────────────────┐
                   │         ESP32            │
                   │                          │
        GPIO21 <───┤ SDA                  3V3 ├───> OLED VCC
        GPIO22 <───┤ SCL                  GND ├───> OLED GND
        GPIO25 <───┤ IN1                      │
        GPIO26 <───┤ IN2                      │
        GPIO32 <───┤ OPEN LIMIT               │
        GPIO33 <───┤ CLOSED LIMIT             │
                   └──────────┬───────────────┘
                              │
                              │ I²C Bus
                              │
        ┌─────────────────────┴──────────────────────┐
        │                  INA219                    │
        │                                             │
        │ SDA <───────────────────────────────────────┘
        │ SCL <───────────────────────────────────────┘
        │ VCC → 3.3V                                  │
        │ GND → GND                                   │
        │ VIN+ ← 12V (+)                              │
        │ VIN− → DRV8871 VM                           │
        └─────────────────────┬──────────────────────┘
                              │
                              │ Motor Power
                              │
        ┌─────────────────────┴──────────────────────┐
        │                 DRV8871                     │
        │                                             │
        │ IN1 ← GPIO25                                │
        │ IN2 ← GPIO26                                │
        │ VM  ← INA219 VIN−                           │
        │ GND → GND                                   │
        │ OUT1 → Motor Lead 1                         │
        │ OUT2 → Motor Lead 2                         │
        └─────────────────────────────────────────────┘
