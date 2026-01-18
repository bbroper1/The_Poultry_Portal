#include "Config.h"
#include <Preferences.h>
#include "Energy.h"

static Preferences prefs;

// Internal storage
static float myLat = 30.30;
static float myLong = -97.37;
static String timezoneStr = "CST6CDT,M3.2.0,M11.1.0";
static String otaPassword = "";

static String telegramToken = "";
static String telegramChatId = "";

static int openOffset = 15;
static int closeOffset = -15;
static int motorTimeoutSec = 45;
static int pinchThresholdmA = 900;

// ---------------------------------------------------------
//  LOAD
// ---------------------------------------------------------
void Config_load() {
    if (prefs.begin("pportal", false)) {

        myLat = prefs.getFloat("lat", myLat);
        myLong = prefs.getFloat("lon", myLong);

        timezoneStr = prefs.getString("tzStr", timezoneStr);
        otaPassword = prefs.getString("otaPass", otaPassword);

        telegramToken  = prefs.getString("botToken", telegramToken);
        telegramChatId = prefs.getString("chatId", telegramChatId);

        openOffset = prefs.getInt("openOff", openOffset);
        closeOffset = prefs.getInt("closeOff", closeOffset);

        motorTimeoutSec = prefs.getInt("motortime", motorTimeoutSec);
        pinchThresholdmA = prefs.getInt("pinch", pinchThresholdmA);

        prefs.end();
    }
}

// ---------------------------------------------------------
//  SAVE
// ---------------------------------------------------------
void Config_save() {
    if (prefs.begin("pportal", false)) {

        prefs.putFloat("lat", myLat);
        prefs.putFloat("lon", myLong);

        prefs.putString("tzStr", timezoneStr);
        prefs.putString("otaPass", otaPassword);

        prefs.putString("botToken", telegramToken);
        prefs.putString("chatId", telegramChatId);

        prefs.putInt("openOff", openOffset);
        prefs.putInt("closeOff", closeOffset);

        prefs.putInt("motortime", motorTimeoutSec);
        prefs.putInt("pinch", pinchThresholdmA);

        prefs.putFloat("todayMAh", Energy_getTodaymAh());

        prefs.end();
    }
}

// ---------------------------------------------------------
//  GETTERS
// ---------------------------------------------------------
float Config_getLat() { return myLat; }
float Config_getLong() { return myLong; }
String Config_getTimezone() { return timezoneStr; }
String Config_getOTAPassword() { return otaPassword; }

String Config_getBotToken() { return telegramToken; }
String Config_getChatID()   { return telegramChatId; }

int Config_getOpenOffset() { return openOffset; }
int Config_getCloseOffset() { return closeOffset; }
int Config_getMotorTimeout() { return motorTimeoutSec; }
int Config_getPinchThreshold() { return pinchThresholdmA; }

// ---------------------------------------------------------
//  SETTERS
// ---------------------------------------------------------
void Config_setLat(float v) { myLat = v; }
void Config_setLong(float v) { myLong = v; }
void Config_setTimezone(const String& tz) { timezoneStr = tz; }
void Config_setOTAPassword(const String& pass) { otaPassword = pass; }

void Config_setBotToken(const String& token) { telegramToken = token; }
void Config_setChatID(const String& id)      { telegramChatId = id; }

void Config_setOpenOffset(int v) { openOffset = v; }
void Config_setCloseOffset(int v) { closeOffset = v; }
void Config_setMotorTimeout(int v) { motorTimeoutSec = v; }
void Config_setPinchThreshold(int v) { pinchThresholdmA = v; }
