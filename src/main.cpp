#define VERSION "4.4.2"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <AsyncTelegram2.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <sunset.h>
#include <Adafruit_INA219.h>
#include <Preferences.h>
#include <WiFiManager.h>
#include <esp_task_wdt.h>

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

// --- PIN DEFINITIONS ---
#define PIN_MOTOR_A       25 
#define PIN_MOTOR_B       26 
#define PIN_LIMIT_OPEN    32
#define PIN_LIMIT_CLOSE   33 
#define PIN_SWITCH_OPEN   14
#define PIN_SWITCH_CLOSE  27
#define HEARTBEAT_LED      2   // loop heartbeat indicator

// --- SETTINGS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MAX_LOGS 10
#define MOTOR_STALL_CURRENT 2000.0  // mA - adjust for your motor
#define MOTOR_TIMEOUT 30000UL       // 30 seconds max runtime
#define VOLTAGE_THRESHOLD 11.5      // Low battery warning
#define CRITICAL_VOLTAGE 10.5       // Critical shutdown voltage
#define TEMP_WARNING 70.0           // °C
#define TEMP_CRITICAL 80.0          // °C
#define WDT_TIMEOUT 60
#define PWM_FREQ 1000
#define PWM_RESOLUTION 8
#define PWM_CH_A 0
#define PWM_CH_B 1

// --- TELEGRAM HANDLER FORWARD DECLARATIONS ---
void handleStatus(TBMessage &msg);
void handleEnergy(TBMessage &msg);
void handleLogs(TBMessage &msg);
void handleOpen(TBMessage &msg);
void handleClose(TBMessage &msg);
void handleAuto(TBMessage &msg);
void handleSettings(TBMessage &msg);
void handleTimezone(TBMessage &msg);
void handleOffsetInput(TBMessage &msg);
void handleResetEnergy(TBMessage &msg);
void handleReboot(TBMessage &msg);
void handleHelp(TBMessage &msg);
void handleUnknown(TBMessage &msg);
void handleTelegram();

// --- GLOBALS ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_INA219 ina219;
WiFiClientSecure secured_client;
AsyncTelegram2 bot(secured_client);
SunSet sun;
Preferences prefs;

float totalUsedMAh = 0;       
float historyMAh[30] = {0};   
unsigned long lastEnergyCalc = 0;
unsigned long bootTime = 0;
bool ignoreFirstTelegramMessage = true;



String eventLogs[MAX_LOGS];
int logIndex = 0;
String TELEGRAM_TOKEN = ""; 
int64_t userid = 0; 
bool telegramEnabled = false;

enum DoorState { OPEN, CLOSED, OPENING, CLOSING, STUCK };
DoorState currentState = CLOSED;
bool remoteOverride = false;

float myLat = 30.30, myLong = -97.37;
String timezoneStr = "CST6CDT,M3.2.0,M11.1.0"; // US Central
int openOffset = 15;  
int closeOffset = -15; 
String nextSunriseStr = "--:--";
bool waitingForOpenOffset = false, waitingForCloseOffset = false;

unsigned long motorStartTime = 0;
float esp32Temp = 0;
bool autoOpenTriggeredToday = false, autoCloseTriggeredToday = false;
bool lowBatAlertSent = false;
bool tempWarningShown = false;
int lastResetDay = -1;
unsigned long lastScreenSwitch = 0;
int currentScreen = 0;  // 0 = main, 1 = power/health
float inputVoltage = 0;
float outputVoltage = 0;
unsigned long totalOpenTime = 0;
unsigned long totalCloseTime = 0;
unsigned int openCycles = 0;
unsigned int closeCycles = 0;
float avgOpenTime = 0;
float avgCloseTime = 0;
unsigned long lastFlashSave = 0;
unsigned long lastBatCheck = 0;
bool displayAwake = true;
unsigned long displayWakeTime = 0;
const unsigned long DISPLAY_TIMEOUT = 60000; // 1 minute


// Gear animation frames
const unsigned char PROGMEM gear_frame_1[] = {
  0x00,0x3C,0x42,0x81,0xA5,0x81,0x99,0x81,
  0x81,0x99,0x81,0xA5,0x81,0x42,0x3C,0x00
};

const unsigned char PROGMEM gear_frame_2[] = {
  0x00,0x3C,0x42,0xA5,0x81,0x99,0x81,0x81,
  0x81,0x81,0x99,0x81,0xA5,0x42,0x3C,0x00
};

const unsigned char PROGMEM gear_frame_3[] = {
  0x00,0x3C,0x42,0x81,0x99,0x81,0xA5,0x81,
  0x81,0xA5,0x81,0x99,0x81,0x42,0x3C,0x00
};

const unsigned char PROGMEM gear_frame_4[] = {
  0x00,0x3C,0x42,0x99,0x81,0xA5,0x81,0x81,
  0x81,0x81,0xA5,0x81,0x99,0x42,0x3C,0x00
};

const unsigned char* gear_frames[] = {
  gear_frame_1,
  gear_frame_2,
  gear_frame_3,
  gear_frame_4
};

// Helper to get current offset for SunSet library
float getCurrentUTCOffset() {
  time_t now = time(nullptr);
  struct tm *gt = gmtime(&now);
  time_t gmt = mktime(gt);
  
  struct tm *lt = localtime(&now);
  time_t loc = mktime(lt);
  
  return (float)(loc - gmt) / 3600.0;
}

// --- MOTOR HELPERS ---
void motorStop() { 
  ledcWrite(PWM_CH_A, 0); 
  ledcWrite(PWM_CH_B, 0); 
  motorStartTime = 0; 
}

void motorOpenRaw(int s) { 
  ledcWrite(PWM_CH_A, s); 
  ledcWrite(PWM_CH_B, 0); 
  if (!motorStartTime) motorStartTime = millis(); 
}

void motorCloseRaw(int s) { 
  ledcWrite(PWM_CH_A, 0); 
  ledcWrite(PWM_CH_B, s); 
  if (!motorStartTime) motorStartTime = millis(); 
}

void motorSoftStart(bool op) { 
  for (int s = 100; s <= 255; s += 15) { 
    if (op) motorOpenRaw(s); 
    else    motorCloseRaw(s); 
    delay(30); 
  } 
}

void motorSoftStop() { 
  for (int s = 255; s >= 0; s -= 20) { 
    if (currentState == OPENING)      motorOpenRaw(s); 
    else if (currentState == CLOSING) motorCloseRaw(s); 
    delay(20); 
  } 
  motorStop(); 
}
void wakeDisplay() {
  displayAwake = true;
  displayWakeTime = millis();
  display.ssd1306_command(SSD1306_DISPLAYON);
}

void sleepDisplay() {
  displayAwake = false;
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void requestOpen() { 
  if (currentState == OPENING || currentState == OPEN) return;
  motorSoftStart(true); 
  currentState = OPENING; 
}

void requestClose() { 
  if (currentState == CLOSING || currentState == CLOSED) return;
  motorSoftStart(false); 
  currentState = CLOSING; 
}

float getBatteryVoltage() { 
  return ina219.getBusVoltage_V(); 
}

float getMotorCurrent() { 
  return abs(ina219.getCurrent_mA()); 
}

void addLog(String event) {
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);
  char buf[20];
  if (ptm && ptm->tm_year > 70) { 
    strftime(buf, sizeof(buf), "[%H:%M]", ptm); 
    eventLogs[logIndex] = String(buf) + " " + event; 
  } else { 
    eventLogs[logIndex] = "[BOOT] " + event; 
  }
  logIndex = (logIndex + 1) % MAX_LOGS;
}

void saveConfig() {
  if (prefs.begin("pportal", false)) {
    prefs.putFloat("lat", myLat); 
    prefs.putFloat("lon", myLong);
    prefs.putString("tzStr", timezoneStr);
    
    if (TELEGRAM_TOKEN != "") {
      prefs.putString("bot_token", TELEGRAM_TOKEN);
    }

    prefs.putInt("openOff", openOffset); 
    prefs.putInt("closeOff", closeOffset);
    prefs.putFloat("todayMAh", totalUsedMAh);
    prefs.putBytes("histMAh", historyMAh, sizeof(historyMAh));
    prefs.end();
  }
}

void loadConfig() {
  if (prefs.begin("pportal", false)) {
    myLat = prefs.getFloat("lat", 30.30); 
    myLong = prefs.getFloat("lon", -97.37);
    timezoneStr = prefs.getString("tzStr", "CST6CDT,M3.2.0,M11.1.0");
    
    String storedToken = prefs.getString("bot_token", "");
    if (storedToken != "") {
      TELEGRAM_TOKEN = storedToken;
    }

    openOffset = prefs.getInt("openOff", 15); 
    closeOffset = prefs.getInt("closeOff", -15);
    totalUsedMAh = prefs.getFloat("todayMAh", 0);

    if (prefs.isKey("histMAh")) {
      size_t storedSize = prefs.getBytesLength("histMAh");
      if (storedSize == sizeof(historyMAh)) {
        prefs.getBytes("histMAh", historyMAh, sizeof(historyMAh));
      }
    }
    prefs.end();
  }

  // Sanitize energy history
  for (int i = 0; i < 30; i++) {
    if (isnan(historyMAh[i]) || isinf(historyMAh[i])) {
      historyMAh[i] = 0;
    }
  }
  if (isnan(totalUsedMAh) || isinf(totalUsedMAh)) {
    totalUsedMAh = 0;
  }

  // Preseed missing history
  bool hasRealData = false;
  bool modified = false;

  for (int i = 0; i < 30; i++) {
    if (historyMAh[i] > 0.1) {
      hasRealData = true;
      break;
    }
  }

  for (int i = 0; i < 30; i++) {
    if (historyMAh[i] < 0.1) {
      historyMAh[i] = 50 + (i * 3);   // 50 → 140 mAh
      modified = true;
    }
  }

  if (!hasRealData) {
    totalUsedMAh = historyMAh[29];
    modified = true;
  }

  if (modified) {
    saveConfig();
    Serial.println("⚠️ Filled missing energy history");
  }
}

// --- SAFETY CHECKS ---
void checkMotorStall() {
  if (motorStartTime == 0) return;
  if (millis() - motorStartTime < 2000) return; // 2s grace period
  
  float current = getMotorCurrent();
  if (current > MOTOR_STALL_CURRENT) {
    motorSoftStop();
    currentState = STUCK;
    addLog("STALL! " + String(current, 0) + "mA 🚨");
    if (telegramEnabled && userid != 0) {
      String msg = "🚨 MOTOR STALL - Current: " + String(current, 0) + "mA\nCheck for obstruction!";
      bot.sendTo(userid, msg.c_str());
    }
  }
}
String getTempLabel(float t) {
  if (t < 60) return "(Normal 🙂)";
  if (t < 70) return "(Warm 😐)";
  if (t < 80) return "(High ⚠️)";
  return "(CRITICAL 🔥)";
}

void checkMotorTimeout() {
  if (motorStartTime > 0 && (millis() - motorStartTime) > MOTOR_TIMEOUT) {
    motorSoftStop();
    currentState = STUCK;
    addLog("TIMEOUT! 🚨");
    if (telegramEnabled && userid != 0) {
      bot.sendTo(userid, "🚨 MOTOR TIMEOUT - Door may be stuck!");
    }
  }
}

void checkTemperature() {
  esp32Temp = (temprature_sens_read() - 32) / 1.8;
  
  if (esp32Temp > TEMP_CRITICAL) {
    motorStop();
    if (currentState == OPENING || currentState == CLOSING) {
      currentState = STUCK;
      addLog("OVERHEAT! 🔥");
      if (telegramEnabled && userid != 0) {
        bot.sendTo(userid, ("🔥 CRITICAL TEMP: " + String(esp32Temp, 1) + "°C - Motor stopped!").c_str());
      }
    }
  } else if (esp32Temp > TEMP_WARNING && !tempWarningShown) {
    addLog("High Temp ⚠️");
    if (telegramEnabled && userid != 0) {
      bot.sendTo(userid, ("⚠️ High temperature: " + String(esp32Temp, 1) + "°C").c_str());
    }
    tempWarningShown = true;
  } else if (esp32Temp < TEMP_WARNING - 5) {
    tempWarningShown = false;
  }
}

void checkBattery() {
  float v = getBatteryVoltage();
  
  if (v < CRITICAL_VOLTAGE) {
    addLog("CRITICAL BATTERY! 🪫");
    if (currentState == OPENING || currentState == CLOSING) {
      motorStop();
      currentState = STUCK;
    }
    if (telegramEnabled && userid != 0 && !lowBatAlertSent) {
      bot.sendTo(userid, ("🪫 CRITICAL: " + String(v, 1) + "V - System halted").c_str());
      lowBatAlertSent = true;
    }
  }
  else if (v < VOLTAGE_THRESHOLD && !lowBatAlertSent) {
    addLog("Low Battery ⚠️");
    if (telegramEnabled && userid != 0) {
      bot.sendTo(userid, ("⚠️ LOW BATTERY: " + String(v, 1) + "V").c_str());
    }
    lowBatAlertSent = true;
  } 
  else if (v > VOLTAGE_THRESHOLD + 0.5) {
    lowBatAlertSent = false;
  }
}

// --- DISPLAY HELPERS ---
String formatEvent(String label, time_t t, int offset) {
  struct tm* ptm = localtime(&t);

  char dateBuf[10];
  sprintf(dateBuf, "%02d/%02d", ptm->tm_mon + 1, ptm->tm_mday);

  int minutes = ptm->tm_hour * 60 + ptm->tm_min;

  String offsetStr = "(";
  offsetStr += (offset >= 0 ? "+" : "");
  offsetStr += String(offset);
  offsetStr += ")";

  return label + " " + String(dateBuf) + ": " +
         String(minutes / 60) + ":" +
         (minutes % 60 < 10 ? "0" : "") + String(minutes % 60) +
         " " + offsetStr;
}

String getSmartNextOpen() {
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);

  // Today’s sunrise
  sun.setCurrentDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
  sun.setPosition(myLat, myLong, getCurrentUTCOffset());
  int sunriseToday = (int)sun.calcSunrise() + openOffset;

  int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

  // If sunrise already passed → use tomorrow
  if (minutesNow >= sunriseToday) {
    time_t tomorrow = now + 86400;
    struct tm* tm2 = localtime(&tomorrow);

    sun.setCurrentDate(tm2->tm_year + 1900, tm2->tm_mon + 1, tm2->tm_mday);
    sun.setPosition(myLat, myLong, getCurrentUTCOffset());
    int sunriseTomorrow = (int)sun.calcSunrise() + openOffset;

    tm2->tm_hour = sunriseTomorrow / 60;
    tm2->tm_min  = sunriseTomorrow % 60;

    return formatEvent("Open", mktime(tm2), openOffset);
  }

  // Otherwise today
  ptm->tm_hour = sunriseToday / 60;
  ptm->tm_min  = sunriseToday % 60;

  return formatEvent("Open", mktime(ptm), openOffset);
}

String getSmartNextClose() {
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);

  // Today’s sunset
  sun.setCurrentDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
  sun.setPosition(myLat, myLong, getCurrentUTCOffset());
  int sunsetToday = (int)sun.calcSunset() + closeOffset;

  int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

  // If sunset already passed → use tomorrow
  if (minutesNow >= sunsetToday) {
    time_t tomorrow = now + 86400;
    struct tm* tm2 = localtime(&tomorrow);

    sun.setCurrentDate(tm2->tm_year + 1900, tm2->tm_mon + 1, tm2->tm_mday);
    sun.setPosition(myLat, myLong, getCurrentUTCOffset());
    int sunsetTomorrow = (int)sun.calcSunset() + closeOffset;

    tm2->tm_hour = sunsetTomorrow / 60;
    tm2->tm_min  = sunsetTomorrow % 60;

    return formatEvent("Close", mktime(tm2), closeOffset);
  }

  // Otherwise today
  ptm->tm_hour = sunsetToday / 60;
  ptm->tm_min  = sunsetToday % 60;

  return formatEvent("Close", mktime(ptm), closeOffset);
}

void showScreenMain() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("COOP v");
  display.println(VERSION);

  float v = getBatteryVoltage();
  int pct = (v >= 13.3) ? 100 : (v >= 13.1) ? 70 : (v >= 12.9) ? 30 : (v >= 12.0) ? 10 : 0;

  display.print("Batt: ");
  display.print(v,1);
  display.print("V ");
  display.print(pct);
  display.println("%");
  display.println(getSmartNextOpen());
  display.println(getSmartNextClose());
  display.print("Door: ");
  switch(currentState) {
    case OPEN: display.println("OPEN"); break;
    case CLOSED: display.println("CLOSED"); break;
    case OPENING: display.println("OPENING"); break;
    case CLOSING: display.println("CLOSING"); break;
    case STUCK: display.println("STUCK"); break;
  }

  display.print("Temp: ");
  display.print(esp32Temp,0);
  display.println("C");

  display.print("Today: ");
  display.print(totalUsedMAh,0);
  display.println("mAh");

  display.display();
}

void showScreenPower() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("POWER & HEALTH");

  display.print("In: ");
  display.print(inputVoltage,2);
  display.print("V  Out: ");
  display.println(outputVoltage,2);

  float eff = (inputVoltage > 0) ? (outputVoltage / inputVoltage) * 100.0 : 0;
  display.print("Eff: ");
  display.print(eff,1);
  display.println("%");

  display.print("Cycles: ");
  display.println(openCycles + closeCycles);

  display.print("Avg Open: ");
  display.print(avgOpenTime,1);
  display.println("s");

  display.print("Avg Close: ");
  display.print(avgCloseTime,1);
  display.println("s");

  display.display();
}

void showGearAnimation() {
  static int frame = 0;
  static unsigned long lastFrame = 0;

  if (millis() - lastFrame > 150) {
    frame = (frame + 1) % 4;
    lastFrame = millis();
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);

  if (currentState == OPENING)
    display.println("OPENING");
  else
    display.println("CLOSING");

  display.drawBitmap(56, 32, gear_frames[frame], 16, 16, SSD1306_WHITE);
  display.display();
}

void updateDisplayManager() {
  if (currentState == OPENING || currentState == CLOSING) {
    showGearAnimation();
    return;
  }

  if (millis() - lastScreenSwitch > 5000) {
    lastScreenSwitch = millis();
    currentScreen = !currentScreen;
  }

  if (currentScreen == 0)
    showScreenMain();
  else
    showScreenPower();
}

// --- WATCHDOG-SAFE BOOT SPLASH ---
void showBootScreenSafe() {
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
  while (millis() - splashStart < 3000) {  // 3 seconds
    //esp_task_wdt_reset();
    delay(20);
  }
}

String getUptime() {
  unsigned long ms = millis();
  unsigned long sec = ms / 1000;
  unsigned long min = sec / 60;
  unsigned long hr  = min / 60;
  unsigned long day = hr / 24;

  sec %= 60;
  min %= 60;
  hr  %= 24;

  char buf[40];
  if (day > 0)
    sprintf(buf, "%lud %luh %lum %lus", day, hr, min, sec);
  else if (hr > 0)
    sprintf(buf, "%luh %lum %lus", hr, min, sec);
  else if (min > 0)
    sprintf(buf, "%lum %lus", min, sec);
  else
    sprintf(buf, "%lus", sec);

  return String(buf);
}
String getHealthLabel() {
  size_t freeHeap = ESP.getFreeHeap();

  if (freeHeap > 200000) return "Excellent 💚";
  if (freeHeap > 150000) return "Good 💙";
  if (freeHeap > 100000) return "Fair 🟡";
  if (freeHeap > 50000)  return "Low 🟠";
  return "Critical 🔴";
}

// --- TELEGRAM HELPERS & HANDLERS ---

ReplyKeyboard buildMainKeyboard() {
  ReplyKeyboard kbd;
  kbd.addButton("/status"); kbd.addButton("/health"); kbd.addRow();
  kbd.addButton("/energy");   kbd.addButton("/logs");  kbd.addRow();
  kbd.addButton("/settings");   kbd.addButton("/help");   kbd.addRow();
  kbd.addButton("/open");
  kbd.addButton("/auto"); kbd.addButton("/close");
  return kbd;
}
ReplyKeyboard buildTimezoneKeyboard() {
  ReplyKeyboard kbd;
  kbd.addButton("/timezone pacific");
  kbd.addButton("/timezone mountain");
  kbd.addRow();
  kbd.addButton("/timezone central");
  kbd.addButton("/timezone eastern");
  kbd.addRow();
  kbd.addButton("/back");
  return kbd;
}

bool pollTelegram(TBMessage &msg) {
  unsigned long start = millis();
  while (millis() - start < 300) {
    if (bot.getNewMessage(msg)) return true;
    delay(5);
  }
  return false;
}

void handleStatus(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  float v = getBatteryVoltage();
  int pct = (v >= 13.3) ? 100 : (v >= 13.1) ? 70 : (v >= 12.9) ? 30 : (v >= 12.0) ? 10 : 0;

  String stateStr;
  switch (currentState) {
    case OPEN: stateStr = "OPEN ✅"; break;
    case CLOSED: stateStr = "CLOSED 🌙"; break;
    case OPENING: stateStr = "OPENING ⚙️"; break;
    case CLOSING: stateStr = "CLOSING ⚙️"; break;
    case STUCK: stateStr = "STUCK ⚠️"; break;
  }

  String m = "📊 *POULTRY PORTAL v" + String(VERSION) + "*\n━━━━━━━━━━━━━━━\n";
  m += "🚪 Door: " + stateStr + "\n";
  m += "🔋 Batt: " + String(v, 1) + "V (" + String(pct) + "%)\n";
  m += "⚡ Today: " + String(totalUsedMAh, 1) + " mAh\n";
  m += "🌡️ Temp: " + String(esp32Temp, 1) + "°C " + getTempLabel(esp32Temp) + "\n";
  m += "📍 Location: " + String(myLat, 4) + ", " + String(myLong, 4) + "\n";
  m += "⏱️ Uptime: " + getUptime() + "\n";
  m += "⚙️ Mode: " + String(remoteOverride ? "MANUAL 🛠️" : "AUTO 🤖") + "\n";
  m += "━━━━━━━━━━━━━━━\n";
  m += "🌅 " + getSmartNextOpen() + "\n";
  m += "🌇 " + getSmartNextClose() + "\n";
  
  bot.sendMessage(msg, m.c_str(), kbd);
}

void handleEnergy(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  // Build data list
  String dp = "";
  for (int i = 29; i >= 0; i--) {
    dp += String(historyMAh[i], 0);
    if (i > 0) dp += ",";
  }

  // Build 7-day rolling average
  String avg = "";
  for (int i = 29; i >= 0; i--) {
    float sum = 0;
    int count = 0;
    for (int j = i; j > i - 7 && j >= 0; j--) {
      sum += historyMAh[j];
      count++;
    }
    float avgVal = sum / count;
    avg += String(avgVal, 0);
    if (i > 0) avg += ",";
  }

  // Build JSON chart with title, gridlines, and 7-day average
  String json = "{"
    "type:'line',"
    "data:{"
      "labels:[30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1],"
      "datasets:["
        "{label:'Daily mAh',data:[" + dp + "],fill:true,"
        "backgroundColor:'rgba(54,162,235,0.2)',"
        "borderColor:'rgb(54,162,235)',"
        "borderWidth:2},"
        "{label:'7-Day Avg',data:[" + avg + "],fill:false,"
        "borderColor:'rgb(255,99,132)',"
        "borderWidth:2,"
        "tension:0.3}"
      "]"
    "},"
    "options:{"
      "plugins:{"
        "title:{display:true,text:'30-Day Energy Usage',font:{size:18}}"
      "},"
      "scales:{"
        "x:{grid:{display:true}},"
        "y:{grid:{display:true}}"
      "}"
    "}"
  "}";

  json.replace(" ", "");

  String chartPng = "https://quickchart.io/chart.png?chart=" + json;

  // Send as photo (no URL text)
  bot.sendPhoto(msg, chartPng.c_str(), "");
}

void handleLogs(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  String logMsg = "📋 *RECENT ACTIVITY*\n━━━━━━━━━━━━━━━\n";
  bool found = false;

  for (int i = 0; i < MAX_LOGS; i++) {
    int idx = (logIndex + i) % MAX_LOGS;
    if (eventLogs[idx] != "") {
      logMsg += eventLogs[idx] + "\n";
      found = true;
    }
  }

  if (!found) logMsg += "No logs yet.";

  bot.sendMessage(msg, logMsg.c_str(), kbd);
}

void handleOpen(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();
  remoteOverride = true;

  if (digitalRead(PIN_LIMIT_OPEN) == LOW) {
    currentState = OPEN;
    bot.sendMessage(msg, "✅ Door is already OPEN", kbd);
  } else {
    requestOpen();
    addLog("Remote Open 📱");
    bot.sendMessage(msg, "🔓 Opening door...", kbd);
  }
}

void handleClose(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();
  remoteOverride = true;

  if (digitalRead(PIN_LIMIT_CLOSE) == LOW) {
    currentState = CLOSED;
    bot.sendMessage(msg, "🌙 Door is already CLOSED", kbd);
  } else {
    requestClose();
    addLog("Remote Close 📱");
    bot.sendMessage(msg, "🔒 Closing door...", kbd);
  }
}

void handleAuto(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();
  remoteOverride = false;

  if (currentState == STUCK) {
    currentState = CLOSED;
    motorStop();
  }

  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now);

  if (!ptm) {
    bot.sendMessage(msg, "🤖 Auto Mode Enabled\n(Time not synced yet)", kbd);
    return;
  }

  sun.setCurrentDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
  sun.setPosition(myLat, myLong, getCurrentUTCOffset());

  int sunrise = (int)sun.calcSunrise() + openOffset;
  int sunset  = (int)sun.calcSunset()  + closeOffset;
  int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

  bool shouldBeOpen = (minutesNow >= sunrise && minutesNow < sunset);

  if (shouldBeOpen && currentState != OPEN) {
    requestOpen();
    addLog("Auto Mode → Correcting to OPEN");
    bot.sendMessage(msg, "🔄 Auto Mode: Opening door to match schedule", kbd);
  }
  else if (!shouldBeOpen && currentState != CLOSED) {
    requestClose();
    addLog("Auto Mode → Correcting to CLOSED");
    bot.sendMessage(msg, "🔄 Auto Mode: Closing door to match schedule", kbd);
  }
  else {
    bot.sendMessage(msg, "🤖 Auto Mode Enabled\nDoor is already in the correct position", kbd);
  }

  addLog("Auto Mode 🤖");
}

float getAverageDailyMAh() {
  float sum = 0;
  int count = 0;

  for (int i = 0; i < 30; i++) {
    if (historyMAh[i] > 0) {
      sum += historyMAh[i];
      count++;
    }
  }

  if (count == 0) return 0;
  return sum / count;
}

float getMonthlyMAh() {
  float sum = 0;
  for (int i = 0; i < 30; i++) {
    sum += historyMAh[i];
  }
  return sum;
}

void handleHealth(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  size_t freeHeap = ESP.getFreeHeap();
  size_t minHeap  = ESP.getMinFreeHeap();
  float vBatt     = getBatteryVoltage();
  float motorA    = getMotorCurrent();
  int rssi        = WiFi.RSSI();
  float avgDaily = getAverageDailyMAh(); 
  float monthly = getMonthlyMAh();
  String uptime   = getUptime();
  esp_reset_reason_t reason = esp_reset_reason();

  String m = "🩺 *SYSTEM HEALTH*\n━━━━━━━━━━━━━━━\n";

  m += "📶 WiFi RSSI: " + String(rssi) + " dBm\n";
  m += "🔋 Battery: " + String(vBatt, 2) + "V\n";
  m += "⚡ Motor Current: " + String(motorA, 0) + " mA\n";
  m += "🌡️ ESP Temp: " + String(esp32Temp, 1) + "°C " + getTempLabel(esp32Temp) + "\n";
  m += "⏱️ Uptime: " + uptime + "\n";
  m += "📈 Avg Daily: " + String(avgDaily, 1) + " mAh/day\n";
  m += "📅 Last 30 Days: " + String(monthly, 1) + " mAh\n";
  m += "🔁 Last Reset: " + String((int)reason) + "\n";
  m += "💾 Free Heap: " + String(freeHeap) + " bytes\n";
  m += "📉 Min Heap: " + String(minHeap) + " bytes\n";
  m += "🧠 Health: " + getHealthLabel() + "\n";

  bot.sendMessage(msg, m.c_str(), kbd);
}

void handleSettings(TBMessage &msg) {
  ReplyKeyboard kbd;

  // First row:
  kbd.addButton("set open offset");
  kbd.addButton("set close offset");

  // Second row:
  kbd.addRow();
  kbd.addButton("/timezone");
  kbd.addButton("/display");

  // Third row:
  kbd.addRow();
  kbd.addButton("/back");

  String s = "⚙️ *SETTINGS*\n━━━━━━━━━━━━━━━\n";
  s += "Open Offset: " + String(openOffset) + " min\n";
  s += "Close Offset: " + String(closeOffset) + " min\n";
  s += "Timezone: " + timezoneStr;

  bot.sendMessage(msg, s.c_str(), kbd);
}


void handleTimezone(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  String val = msg.text.substring(10);
  val.trim();
  String tzName = "";

  if (val.equalsIgnoreCase("central")) {
    timezoneStr = "CST6CDT,M3.2.0,M11.1.0";
    tzName = "Central";
  }
  else if (val.equalsIgnoreCase("eastern")) {
    timezoneStr = "EST5EDT,M3.2.0,M11.1.0";
    tzName = "Eastern";
  }
  else if (val.equalsIgnoreCase("mountain")) {
    timezoneStr = "MST7MDT,M3.2.0,M11.1.0";
    tzName = "Mountain";
  }
  else if (val.equalsIgnoreCase("pacific")) {
    timezoneStr = "PST8PDT,M3.2.0,M11.1.0";
    tzName = "Pacific";
  }
  else {
    bot.sendMessage(msg, "Usage: /timezone [Central|Eastern|Mountain|Pacific]", kbd);
    return;
  }

  setenv("TZ", timezoneStr.c_str(), 1);
  tzset();
  saveConfig();

  bot.sendMessage(msg, ("🌐 Timezone: " + tzName).c_str(), kbd);
}

void handleOffsetInput(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();
  int val = msg.text.toInt();

  if (waitingForOpenOffset) {
    openOffset = val;
    waitingForOpenOffset = false;
    saveConfig();
    bot.sendMessage(msg, ("✅ Open offset set to " + String(openOffset) + " minutes").c_str(), kbd);
  }
  else if (waitingForCloseOffset) {
    closeOffset = val;
    waitingForCloseOffset = false;
    saveConfig();
    bot.sendMessage(msg, ("✅ Close offset set to " + String(closeOffset) + " minutes").c_str(), kbd);
  }
}

void handleResetEnergy(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  for (int i = 0; i < 30; i++) historyMAh[i] = 0;
  totalUsedMAh = 0;
  saveConfig();

  bot.sendMessage(msg, "🔋 Energy history reset", kbd);
}

void handleReboot(TBMessage &msg) {
  bot.sendMessage(msg, "🔄 Rebooting...");
  delay(1000);
  ESP.restart();
}

void handleHelp(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();

  String h = "📖 *HELP GUIDE*\n━━━━━━━━━━━━━━━\n";
  h += "/status – Full system report\n";
  h += "/health – System diagnostics (heap, RSSI, temp, uptime)\n";
  h += "/energy – 30‑day usage chart with 7‑day average\n";
  h += "/logs – Recent activity\n";
  h += "/open – Open the door\n";
  h += "/close – Close the door\n";
  h += "/auto – Return to auto mode\n";
  h += "/settings – Adjust offsets & timezone\n";
  h += "/resetenergy – Reset energy history\n";
  h += "/reboot – Restart the controller\n\n";
  h += "📍 Send your location to update sunrise/sunset times";

  bot.sendMessage(msg, h.c_str(), kbd);
}

void handleUnknown(TBMessage &msg) {
  ReplyKeyboard kbd = buildMainKeyboard();
  String reply = "❓ Unknown command: '" + msg.text + "'\nTry /help";
  bot.sendMessage(msg, reply.c_str(), kbd);
}

void handleTelegram() {
  // HARD BOOT LOCKOUT: ignore Telegram for first 8 seconds
  if (millis() - bootTime < 8000) return;

  if (!telegramEnabled) return;

  TBMessage msg;
  if (!pollTelegram(msg)) return;

  // Ignore the first Telegram message after reboot
  if (ignoreFirstTelegramMessage) {
      ignoreFirstTelegramMessage = false;
      return;
  }

  userid = msg.sender.id;
  String text = msg.text;
  text.toLowerCase();

  // Handle location messages
  if (text.length() == 0 &&
      msg.location.latitude != 0.0 &&
      msg.location.longitude != 0.0 &&
      abs(msg.location.latitude) > 0.1 &&
      abs(msg.location.longitude) > 0.1) {

    myLat = msg.location.latitude;
    myLong = msg.location.longitude;
    sun.setPosition(myLat, myLong, getCurrentUTCOffset());
    saveConfig();

    ReplyKeyboard kbd = buildMainKeyboard();
    String locMsg = "📍 Location Updated!\nLat: " + String(myLat, 4) +
                    "\nLong: " + String(myLong, 4);
    bot.sendMessage(msg, locMsg.c_str(), kbd);
    addLog("Location Updated 📍");
    return;
  }

  // Handle offset input mode
  if (waitingForOpenOffset || waitingForCloseOffset) {
    handleOffsetInput(msg);
    return;
  }

// Command routing
if (text == "/status" || text == "/start") handleStatus(msg);
else if (text == "/energy") handleEnergy(msg);
else if (text == "/logs") handleLogs(msg);
else if (text == "/open") handleOpen(msg);
else if (text == "/close") handleClose(msg);
else if (text == "/auto") handleAuto(msg);
else if (text == "/settings") handleSettings(msg);
else if (text == "/back") {
    ReplyKeyboard kbd = buildMainKeyboard();
    bot.sendMessage(msg, "⬅️ Back to main menu", kbd);
}

// Timezone button → show choices
else if (text == "/timezone") {
    ReplyKeyboard kbd = buildTimezoneKeyboard();
    bot.sendMessage(msg, "Choose your timezone:", kbd);
}

// Timezone command with argument
else if (text.startsWith("/timezone ")) handleTimezone(msg);

// Reset, reboot, help, health
else if (text == "/resetenergy") handleResetEnergy(msg);
else if (text == "/reboot") handleReboot(msg);
else if (text == "/help") handleHelp(msg);
else if (text == "/health") handleHealth(msg);

// Offset buttons
else if (text == "set open offset") {
    waitingForOpenOffset = true;
    waitingForCloseOffset = false;
    bot.sendMessage(msg, "Send new *OPEN* offset in minutes (e.g. 15 or -10):");
}
else if (text == "set close offset") {
    waitingForCloseOffset = true;
    waitingForOpenOffset = false;
    bot.sendMessage(msg, "Send new *CLOSE* offset in minutes (e.g. -15):");
}
else if (text == "/display") {
    wakeDisplay();
    ReplyKeyboard kbd = buildMainKeyboard();
    bot.sendMessage(msg, "🖥️ Display turned on for 1 minute.", kbd);
}
else handleUnknown(msg);
}
// --- SUNRISE/SUNSET AUTOMATION ---
void checkSunriseSunset() {
  if (remoteOverride) return;
  time_t now = time(nullptr); 
  struct tm* ptm = localtime(&now);
  if (!ptm || ptm->tm_year < 120) return;

  sun.setCurrentDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
  sun.setPosition(myLat, myLong, getCurrentUTCOffset());
  
  int r = (int)sun.calcSunrise() + openOffset;
  int s = (int)sun.calcSunset()  + closeOffset;
  int currentMin = ptm->tm_hour * 60 + ptm->tm_min;

  if (currentMin < r) 
    nextSunriseStr = "Open: "  + String(r/60) + ":" + String(r%60<10?"0":"") + String(r%60);
  else if (currentMin < s) 
    nextSunriseStr = "Close: " + String(s/60) + ":" + String(s%60<10?"0":"") + String(s%60);
  else 
    nextSunriseStr = "Closed for night";

  if (currentMin >= r && currentMin < r + 5 && !autoOpenTriggeredToday && currentState == CLOSED) {
    requestOpen(); 
    autoOpenTriggeredToday = true; 
    addLog("Auto Open 🌅");
  }
  if (currentMin >= s && currentMin < s + 5 && !autoCloseTriggeredToday && currentState == OPEN) {
    requestClose(); 
    autoCloseTriggeredToday = true; 
    addLog("Auto Close 🌇");
  }

  if (ptm->tm_mday != lastResetDay) { 
    lastResetDay = ptm->tm_mday;
    autoOpenTriggeredToday = false; 
    autoCloseTriggeredToday = false; 
    
    for (int i = 29; i > 0; i--) {
      historyMAh[i] = historyMAh[i-1];
    }
    historyMAh[0] = totalUsedMAh; 
    totalUsedMAh = 0; 
    saveConfig();
    addLog("Day Reset 📅");
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n🐔 PoultryPortal v" + String(VERSION) + " Starting...");
  bootTime = millis();

  esp_reset_reason_t reason = esp_reset_reason();
  Serial.print("Boot reason: ");
  Serial.println((int)reason);
  if (reason == ESP_RST_TASK_WDT) {
    addLog("Recovered from watchdog reset ⚠️");
  }

  pinMode(PIN_MOTOR_A, OUTPUT);
  pinMode(PIN_MOTOR_B, OUTPUT);
  pinMode(PIN_LIMIT_OPEN, INPUT_PULLUP);
  pinMode(PIN_LIMIT_CLOSE, INPUT_PULLUP);
  pinMode(PIN_SWITCH_OPEN, INPUT_PULLUP);
  pinMode(PIN_SWITCH_CLOSE, INPUT_PULLUP);
  pinMode(HEARTBEAT_LED, OUTPUT);
  digitalWrite(HEARTBEAT_LED, LOW);

  loadConfig();

  WiFiManager wm;
  wm.setConnectTimeout(20);
  if (!wm.autoConnect("PoultryPortal_AP")) {
    Serial.println("❌ WiFi failed");
    delay(3000);
    ESP.restart();
  }
  Serial.println("✅ WiFi connected");

  Wire.begin(21, 22);
  Wire.setClock(400000);
  delay(50);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    delay(200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("⚠️ Display init failed");
    }
  }
  Serial.println("✅ Display ready");

  Serial.println("Initializing INA219...");
  if (!ina219.begin(&Wire)) {
    Serial.println("⚠️ INA219 init failed");
  } else {
    Serial.println("✅ INA219 ready");
  }

  wakeDisplay();  // Display stays on for first minute

  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", timezoneStr.c_str(), 1);
  tzset();

  time_t now = time(nullptr);
  int retries = 0;
  while (now < 100000 && retries < 50) {
    delay(100);
    now = time(nullptr);
    retries++;
  }

  ledcAttachChannel(PIN_MOTOR_A, PWM_FREQ, PWM_RESOLUTION, PWM_CH_A);
  ledcAttachChannel(PIN_MOTOR_B, PWM_FREQ, PWM_RESOLUTION, PWM_CH_B);
  motorStop();

  secured_client.setInsecure();
  if (TELEGRAM_TOKEN.length() > 10) {
    telegramEnabled = true;
    bot.setTelegramToken(TELEGRAM_TOKEN.c_str());
    Serial.println("✅ Telegram enabled");
  } else {
    Serial.println("⚠️ No Telegram token");
  }

  ArduinoOTA.setHostname("PoultryPortal");
  ArduinoOTA.setPassword("chickentender1");
  ArduinoOTA.begin();

  lastEnergyCalc = millis();

  addLog("Online 🚀");
  Serial.println("=== BOOT COMPLETE ===\n");

  // AUTO MODE BOOT CHECK
  if (!remoteOverride) {
    struct tm* ptm = localtime(&now);
    if (ptm) {
      sun.setCurrentDate(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
      sun.setPosition(myLat, myLong, getCurrentUTCOffset());

      int sunrise = (int)sun.calcSunrise() + openOffset;
      int sunset  = (int)sun.calcSunset()  + closeOffset;
      int minutesNow = ptm->tm_hour * 60 + ptm->tm_min;

      if (minutesNow >= sunrise && minutesNow < sunset) {
        if (digitalRead(PIN_LIMIT_OPEN) == HIGH) {
          requestOpen();
          addLog("Auto Boot → Opening (should be open)");
        }
      } else {
        if (digitalRead(PIN_LIMIT_CLOSE) == HIGH) {
          requestClose();
          addLog("Auto Boot → Closing (should be closed)");
        }
      }
    }
  }

  showBootScreenSafe();
  bootTime = millis();

}

// --- LOOP ---
void loop() {
  static unsigned long lastBeat = 0;
  if (millis() - lastBeat > 1000) {
    digitalWrite(HEARTBEAT_LED, !digitalRead(HEARTBEAT_LED));
    lastBeat = millis();
  }

  float dt_h = (millis() - lastEnergyCalc) / 3600000.0;
  float current = getMotorCurrent();
  if (isnan(current) || isinf(current)) current = 0;
  totalUsedMAh += current * dt_h;
  lastEnergyCalc = millis();

  if (millis() - lastFlashSave > 1800000) {
    lastFlashSave = millis();
    saveConfig();
  }

  if (millis() - lastBatCheck > 60000) {
    lastBatCheck = millis();
    checkBattery();
  }

  checkMotorStall();
  checkMotorTimeout();
  checkTemperature();
  checkSunriseSunset();
  handleTelegram();
  ArduinoOTA.handle();

  static unsigned long lastSwitchTime = 0;
  if (millis() - lastSwitchTime > 500) {
    if (digitalRead(PIN_SWITCH_OPEN) == LOW && currentState != OPENING && currentState != OPEN) {
      remoteOverride = true;
      requestOpen();
      addLog("Manual Open 🛠️");
      lastSwitchTime = millis();
    }
    else if (digitalRead(PIN_SWITCH_CLOSE) == LOW && currentState != CLOSING && currentState != CLOSED) {
      remoteOverride = true;
      requestClose();
      addLog("Manual Close 🛠️");
      lastSwitchTime = millis();
    }
  }

  if (digitalRead(PIN_LIMIT_OPEN) == LOW && currentState == OPENING) {
    motorSoftStop();
    currentState = OPEN;
    addLog("Door Opened ✅");
    wakeDisplay();
    if (telegramEnabled && userid != 0) 
        bot.sendTo(userid, "✅ Door is now OPEN");
  }

  else if (digitalRead(PIN_LIMIT_OPEN) == LOW && currentState == STUCK) {
    motorStop();
    currentState = OPEN;
    addLog("Recovered: OPEN");
  }

  if (digitalRead(PIN_LIMIT_CLOSE) == LOW && currentState == CLOSING) {
    motorSoftStop();
    currentState = CLOSED;
    addLog("Door Closed 🌙");
    wakeDisplay();
    if (telegramEnabled && userid != 0) 
        bot.sendTo(userid, "🌙 Door is now CLOSED");
  }
  else if (digitalRead(PIN_LIMIT_CLOSE) == LOW && currentState == STUCK) {
    motorStop();
    currentState = CLOSED;
    addLog("Recovered: CLOSED");
  }

  // OLED auto-sleep
  if (displayAwake && millis() - displayWakeTime > DISPLAY_TIMEOUT) {
      sleepDisplay();
  }

  updateDisplayManager();
}
