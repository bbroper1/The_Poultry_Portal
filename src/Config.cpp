#include "Config.h"
#include <Preferences.h>
#include "Energy.h"   // for Energy_getTodaymAh()

static Preferences prefs;

// Internal storage
static float myLat = 30.30;
static float myLong = -97.37;
static String timezoneStr = "CST6CDT,M3.2.0,M11.1.0";
static String otaPassword = "";
static String telegramToken = "";

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

        String storedToken = prefs.getString("bot_token", "");
        if (storedToken.length() > 0)
            telegramToken = storedToken;

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

        if (telegramToken.length() > 0)
            prefs.putString("bot_token", telegramToken);

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
String Config_getTelegramToken() { return telegramToken; }
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
void Config_setTelegramToken(const String& token) { telegramToken = token; }
void Config_setOpenOffset(int v) { openOffset = v; }
void Config_setCloseOffset(int v) { closeOffset = v; }
void Config_setMotorTimeout(int v) { motorTimeoutSec = v; }
void Config_setPinchThreshold(int v) { pinchThresholdmA = v; }
