#include "modules/display/DisplayTask.h"
#include "modules/system/SystemStatus.h"
#include "modules/scheduler/SchedulerTask.h"
#include "modules/motor/MotorTask.h"
#include "modules/sensors/SensorModule.h"
#include "modules/battery/BatteryModule.h"
#include "modules/energy/EnergyModule.h"
#include "modules/config/Config.h"
#include "modules/system/Logging.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---------------------------------------------------------
// CONTEXT
// ---------------------------------------------------------
struct DisplayContext {
    Adafruit_SSD1306 display;
    bool ok;
    bool awake;
    TickType_t lastSwitch;
    int screen;

    DisplayContext()
        : display(128, 64, &Wire, -1),
          ok(false),
          awake(true),
          lastSwitch(0),
          screen(0)
    {}
};

static DisplayContext* ctx = nullptr;
static TaskHandle_t s_displayTaskHandle = nullptr;

// ---------------------------------------------------------
// BATTERY ICON (same one you already used)
// ---------------------------------------------------------
static void drawBatteryIcon(Adafruit_SSD1306& d, float volts) {
    int level = 0;
    if (volts >= 12.4) level = 4;
    else if (volts >= 12.1) level = 3;
    else if (volts >= 11.9) level = 2;
    else if (volts >= 11.7) level = 1;
    else level = 0;

    d.drawRect(0, 0, 20, 10, SSD1306_WHITE);
    d.fillRect(20, 3, 2, 4, SSD1306_WHITE);

    int fill = level * 4;
    if (fill > 0)
        d.fillRect(2, 2, fill, 6, SSD1306_WHITE);
}

// ---------------------------------------------------------
// HELPERS
// ---------------------------------------------------------
static String extractTimeOnly(const String& smart) {
    int colon = smart.indexOf(':');
    int space = smart.indexOf(' ', colon + 2);
    if (colon < 0 || space < 0) return "--:--";
    return smart.substring(colon + 2, space);
}

// ---------------------------------------------------------
// MAIN SCREEN
// ---------------------------------------------------------
static void drawMain(DisplayContext* c) {
    SystemStatus s = SystemStatus_get();

    String openTime  = extractTimeOnly(Scheduler_getNextOpen());
    String closeTime = extractTimeOnly(Scheduler_getNextClose());

    c->display.clearDisplay();
    c->display.setTextSize(1);
    c->display.setTextColor(SSD1306_WHITE);

    // Battery icon
    drawBatteryIcon(c->display, s.batteryVoltage);

    // Voltage
    c->display.setCursor(26, 0);
    c->display.print(s.batteryVoltage, 1);
    c->display.print("V");

    // Auto/Manual
    c->display.setCursor(70, 0);
    c->display.print(Config_getAutoMode() ? "AUTO" : "MAN");

    // Simulation
    if (Config_isSimulatedHardware()) {
        c->display.setCursor(100, 0);
        c->display.print("SIM");
    }

    // Temperature
    c->display.setCursor(0, 14);
    c->display.print("Temp ");
    c->display.print((int)s.temperatureC);
    c->display.print("C");

    // Open / Close (two rows)
    c->display.setCursor(0, 26);
    c->display.print("Open   ");
    c->display.print(openTime);

    c->display.setCursor(0, 36);
    c->display.print("Close  ");
    c->display.print(closeTime);

    // Door state
    c->display.setCursor(0, 48);
    c->display.print("Door   ");
    switch (s.doorState) {
        case M_OPEN:    c->display.print("OPEN"); break;
        case M_CLOSED:  c->display.print("CLOSED"); break;
        case M_OPENING: c->display.print("OPENING"); break;
        case M_CLOSING: c->display.print("CLOSING"); break;
        case M_STUCK:   c->display.print("STUCK"); break;
    }

    // Version bottom-right
    c->display.setCursor(80, 54);
    c->display.print("v");
    c->display.print(VERSION);

    c->display.display();
}

// ---------------------------------------------------------
// INFO SCREEN
// ---------------------------------------------------------
static void drawInfo(DisplayContext* c) {
    SystemStatus s = SystemStatus_get();

    int sr, ss;
    Scheduler_calcLocalSunTimes(sr, ss);

    c->display.clearDisplay();
    c->display.setTextSize(1);
    c->display.setTextColor(SSD1306_WHITE);

    // Sunrise
    c->display.setCursor(0, 0);
    c->display.print("Sunrise ");
    c->display.print(sr / 60);
    c->display.print(":");
    if (sr % 60 < 10) c->display.print("0");
    c->display.println(sr % 60);

    // Sunset
    c->display.print("Sunset  ");
    c->display.print(ss / 60);
    c->display.print(":");
    if (ss % 60 < 10) c->display.print("0");
    c->display.println(ss % 60);

    // Offsets
    c->display.print("OpenOff  ");
    c->display.print(Config_getOpenOffset());
    c->display.println("m");

    c->display.print("CloseOff ");
    c->display.print(Config_getCloseOffset());
    c->display.println("m");

    // Energy
    c->display.print("Energy   ");
    c->display.print(Energy_getTodaymAh(), 1);
    c->display.println("mAh");

    // Cycles
    c->display.print("Cycles   ");
    c->display.println(s.openCycles + s.closeCycles);

    // Health
    c->display.print("Health   ");
    c->display.println(SystemStatus_getHealthLabel(s));

    c->display.display();
}

// ---------------------------------------------------------
// OPENING / CLOSING SCREEN
// ---------------------------------------------------------
static void drawMotion(DisplayContext* c, const char* label) {
    c->display.clearDisplay();
    c->display.setTextSize(2);
    c->display.setTextColor(SSD1306_WHITE);

    int16_t x1, y1;
    uint16_t w, h;
    c->display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);

    int x = (128 - w) / 2;
    int y = (64 - h) / 2;

    c->display.setCursor(x, y);
    c->display.print(label);

    c->display.display();
}

// ---------------------------------------------------------
// TASK LOOP
// ---------------------------------------------------------
static void DisplayTask(void* pv) {
    DisplayContext* c = ctx;
    const TickType_t tick = pdMS_TO_TICKS(250);
    TickType_t lastWake = xTaskGetTickCount();

    // Boot screen
    c->display.clearDisplay();
    c->display.setTextSize(2);
    c->display.setCursor(10, 20);
    c->display.println("POULTRY");
    c->display.setCursor(20, 40);
    c->display.println("PORTAL");
    c->display.display();
    vTaskDelay(pdMS_TO_TICKS(1500));

    for (;;) {
        vTaskDelayUntil(&lastWake, tick);

        if (!c->ok || !c->awake)
            continue;

        MotorDoorState st = Motor_getState();
        if (st == M_OPENING) {
            drawMotion(c, "OPENING");
            continue;
        }
        if (st == M_CLOSING) {
            drawMotion(c, "CLOSING");
            continue;
        }

        // Auto-switch every 5 seconds
        TickType_t now = xTaskGetTickCount();
        if (now - c->lastSwitch > pdMS_TO_TICKS(5000)) {
            c->lastSwitch = now;
            c->screen = !c->screen;
        }

        if (c->screen == 0)
            drawMain(c);
        else
            drawInfo(c);
    }
}

// ---------------------------------------------------------
// PUBLIC API
// ---------------------------------------------------------
void Display_init() {}
void Display_showBoot() {}
void Display_update() {}

void Display_wake() {
    if (!ctx) return;
    ctx->awake = true;
    ctx->display.ssd1306_command(SSD1306_DISPLAYON);
}

void Display_sleep() {
    if (!ctx) return;
    ctx->awake = false;
    ctx->display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void DisplayTask_begin() {
    if (ctx) return;

    ctx = new DisplayContext();

    if (!ctx->display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        addLog("Display init failed");
        ctx->ok = false;
        return;
    }

    ctx->ok = true;
    ctx->display.clearDisplay();
    ctx->display.setTextColor(SSD1306_WHITE);
    ctx->display.display();

    addLog("Display initialized");

    xTaskCreatePinnedToCore(
        DisplayTask,
        "DisplayTask",
        4096,
        nullptr,
        1,
        &s_displayTaskHandle,
        0
    );
}