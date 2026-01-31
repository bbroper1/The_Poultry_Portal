#include "Display.h"
#include "SystemStatus.h"
#include "Scheduler.h"
#include "Motor.h"
#include "Battery.h"
#include "Energy.h"
#include "Temperature.h"
#include "Config.h"

#include <Adafruit_GFX.h>
#include <WebSerial.h>
#include <Adafruit_SSD1306.h>

// ---------------------------------------------------------
//  INTERNAL STATE
// ---------------------------------------------------------
static constexpr int SCREEN_WIDTH = 128;
static constexpr int SCREEN_HEIGHT = 64;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

static bool displayOk = false;
static bool displayAwake = true;
static unsigned long displayWakeTime = 0;

static unsigned long lastScreenSwitch = 0;
static int currentScreen = 0;

// ---------------------------------------------------------
//  INIT / POWER CONTROL
// ---------------------------------------------------------

void Display_init() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("⚠️ Display init failed!");
        WebSerial.println("⚠️ Display init failed!");
        displayOk = false;
        return;
    }

    displayOk = true;
    display.clearDisplay();
    display.display();
}

void Display_wake() {
    if (!displayOk) return;

    displayAwake = true;
    displayWakeTime = millis();
    display.ssd1306_command(SSD1306_DISPLAYON);
}

void Display_sleep() {
    if (!displayOk) return;

    displayAwake = false;
    display.ssd1306_command(SSD1306_DISPLAYOFF);
}

// ---------------------------------------------------------
//  BOOT SCREEN
// ---------------------------------------------------------

void Display_showBoot() {
    if (!displayOk) return;

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 10);
    display.println("POULTRY");

    display.setCursor(20, 32);
    display.println("PORTAL");

    display.setTextSize(1);
    display.setCursor(40, 52);
    display.print("v");
    display.println(VERSION);
    display.display();

    unsigned long splashStart = millis();
    while (millis() - splashStart < 3000) {
        delay(20);
    }
}

// ---------------------------------------------------------
//  MAIN SCREEN (uses SystemStatus)
// ---------------------------------------------------------

void Display_showMain() {
    if (!displayOk) return;

    SystemStatus s = SystemStatus_get();

    display.clearDisplay();
    display.setTextSize(1);

    // Battery
    display.setCursor(0, 0);
    display.print("Batt:");
    display.print(s.batteryVoltage, 1);
    display.print("V");

    // Temperature
    display.setCursor(80, 0);
    display.print("Temp:");
    display.print((int)s.temperatureC);
    display.print("C");

    // Scheduler
    display.setCursor(0, 16);
    display.print("Open:");
    display.print(Scheduler_getNextOpen());

    display.setCursor(80, 16);
    display.print("Close:");
    display.print(Scheduler_getNextClose());

    // Door state
    display.setCursor(0, 32);
    display.print("Door:");
    switch (s.doorState) {
        case M_OPEN:    display.print("OPEN"); break;
        case M_CLOSED:  display.print("CLOSED"); break;
        case M_OPENING: display.print("OPENING"); break;
        case M_CLOSING: display.print("CLOSING"); break;
        case M_STUCK:   display.print("STUCK"); break;
    }

    // Energy
    display.setCursor(80, 32);
    display.print("Today:");
    display.print(Energy_getTodaymAh());
    display.print("mAh");

    // Version
    display.setCursor(0, 48);
    display.print("COOP v");
    display.print(VERSION);

    display.display();
}

// ---------------------------------------------------------
//  POWER / HEALTH SCREEN (cleaned)
// ---------------------------------------------------------

void Display_showPower() {
    if (!displayOk) return;

    SystemStatus s = SystemStatus_get();

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("POWER & HEALTH");

    // Battery
    display.print("Batt: ");
    display.print(s.batteryVoltage, 2);
    display.println("V");

    // Temperature
    display.print("Temp: ");
    display.print(s.temperatureC, 1);
    display.println("C");

    // Cycles
    display.print("Cycles: ");
    display.println(s.openCycles + s.closeCycles);

    // Health label
    display.print("Health: ");
    display.println(SystemStatus_getHealthLabel(s));

    display.display();
}

// ---------------------------------------------------------
//  DISPLAY MANAGER
// ---------------------------------------------------------

void Display_update() {
    if (!displayOk) return;

    // Show OPENING/CLOSING animation
    if (Motor_getState() == M_OPENING || Motor_getState() == M_CLOSING) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);

        if (Motor_getState() == M_OPENING)
            display.println("OPENING");
        else
            display.println("CLOSING");

        display.display();
        return;
    }

    // Auto-switch every 5 seconds
    if (millis() - lastScreenSwitch > 5000) {
        lastScreenSwitch = millis();
        currentScreen = !currentScreen;
    }

    if (currentScreen == 0)
        Display_showMain();
    else
        Display_showPower();
}
#include "Display.h"
#include "Logging.h"

void Display_begin() {
    Display_init();
    Display_showBoot();
    Display_wake();
    addLog("Display initialized 🖥️");
}
