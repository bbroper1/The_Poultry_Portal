🪛 PoultryPortal Wiring Guide (v2.1.0)
For Dual‑Relay Polarity‑Reversing Motor Driver (DPDT Relay Board)
(Amazon ASIN B099MRT96C)

This guide reflects your actual hardware:

ESP32

DPDT relay motor reversing board

INA219 current sensor

SSD1306 OLED

Limit switches

12V motor + battery

All grounds must be tied together.

🧠 ESP32 → Peripheral Overview
Code
ESP32
├── OLED Display (I²C)
├── INA219 Current Sensor (I²C)
├── DPDT Relay Motor Reversing Board
└── Limit Switches (Open & Closed)
The relay board handles polarity reversal by switching motor leads.

🖥 OLED Display Wiring (SSD1306, I²C)
OLED Pin	ESP32 Pin
VCC	3.3V
GND	GND
SDA	GPIO 21
SCL	GPIO 22


OLED shares I²C with INA219.

🔌 INA219 Current Sensor Wiring
Logic connections:

INA219 Pin	ESP32 Pin
VCC	3.3V
GND	GND
SDA	GPIO 21
SCL	GPIO 22


Power measurement path:

Code
12V (+) → INA219 VIN+
INA219 VIN− → Relay Board Motor Power Input (+)
This lets INA219 measure:

Motor current

Motor voltage

Pinch detection

⚙️ DPDT Relay Motor Driver Wiring
This is the important part — replacing DRV8871 wiring
Your relay board has:

IN1 → Relay A control

IN2 → Relay B control

COM / NO / NC → Motor polarity switching

12V motor power input

GND

ESP32 → Relay Inputs
Relay Board Pin	ESP32 Pin
IN1	GPIO 25
IN2	GPIO 26
VCC	5V
GND	GND


Your board uses 5V logic, but ESP32 GPIO can safely drive it because the relay input is opto‑isolated and low‑current.

Motor Power Wiring
Code
12V (+) → INA219 VIN+
INA219 VIN− → Relay Board Motor + Input
Relay Board Motor – Input → 12V (–)
Motor Output Wiring
Code
Relay OUT1 → Motor lead 1
Relay OUT2 → Motor lead 2
The relay board flips polarity by switching COM/NO/NC internally.

🚪 Limit Switch Wiring (Active‑Low)
Door Open Switch
Code
COM → GND
NO  → GPIO 32
Door Closed Switch
Code
COM → GND
NO  → GPIO 33
ESP32 internal pull‑ups keep pins HIGH until pressed.

🔋 Power Wiring
Code
ESP32 VIN/5V → 5V supply or USB
ESP32 GND   → GND

12V (+)     → INA219 VIN+
INA219 VIN− → Relay Board Motor Power Input (+)
Relay GND   → GND
ESP32 GND   → GND
OLED GND    → GND
INA219 GND  → GND
All grounds must be common.

🧩 Full Wiring Diagram (ASCII)
Code
                   ┌──────────────────────────┐
                   │          ESP32           │
                   │                          │
        GPIO21 <───┤ SDA                  3V3 ├───> OLED VCC
        GPIO22 <───┤ SCL                  GND ├───> OLED GND
        GPIO25 <───┤ Relay IN1                │
        GPIO26 <───┤ Relay IN2                │
        GPIO32 <───┤ OPEN LIMIT               │
        GPIO33 <───┤ CLOSED LIMIT             │
                   └──────────┬───────────────┘
                              │
                              │ I²C Bus
                              │
        ┌─────────────────────┴──────────────────────┐
        │                  INA219                    │
        │                                            │
        │ SDA <──────────────────────────────────────┘
        │ SCL <──────────────────────────────────────┘
        │ VCC → 3.3V                                 │
        │ GND → GND                                  │
        │ VIN+ ← 12V (+)                             │
        │ VIN− → Relay Board Motor + Input           │
        └─────────────────────┬──────────────────────┘
                              │
                              │ Motor Power
                              │
        ┌─────────────────────┴──────────────────────┐
        │      DPDT Relay Motor Reversing Board      │
        │                                             │
        │ IN1 ← GPIO25                                │
        │ IN2 ← GPIO26                                │
        │ VCC ← 5V                                    │
        │ GND → GND                                   │
        │ Motor + Input ← INA219 VIN−                 │
        │ Motor – Input → 12V (–)                     │
        │ OUT1 → Motor Lead 1                         │
        │ OUT2 → Motor Lead 2                         │
        └─────────────────────────────────────────────┘
🧪 Notes & Best Practices
Relay boards draw more current than DRV8871 — ensure 5V supply is stable

Keep motor wires twisted to reduce relay arcing noise

Add a flyback diode across the motor if not already built‑in

If relays chatter, add a 470µF cap on the 5V rail

INA219 must be in the positive motor power path