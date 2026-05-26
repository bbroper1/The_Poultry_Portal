#include "modules/config/Config.h"
#include <Preferences.h>

static Preferences prefs;

// ---------------------------------------------------------
// Internal storage
// ---------------------------------------------------------
static float  s_lat            = 30.30f;
static float  s_long           = -97.37f;
static String s_timezone       = "CST6CDT,M3.2.0,M11.1.0";
static String s_otaPassword    = "admin";

static String s_botToken       = "";
static String s_chatID         = "";

static int s_openOffset        = 0;
static int s_closeOffset       = 0;
static int s_motorTimeout      = 8000;
static int s_pinchThreshold    = 300;

static bool s_autoMode         = false;
static bool s_simulatedHardware = false;   // NEW

// ---------------------------------------------------------
// Load all settings
// ---------------------------------------------------------
void Config_load() {
    prefs.begin("config", false);

    s_lat         = prefs.getFloat("lat", 30.30f);
    s_long        = prefs.getFloat("long", -97.37f);
    s_timezone    = prefs.getString("tz", "CST6CDT,M3.2.0,M11.1.0");
    s_otaPassword = prefs.getString("ota", "admin");

    s_botToken    = prefs.getString("bot", "");
    s_chatID      = prefs.getString("chat", "");

    s_openOffset     = prefs.getInt("openOff", 0);
    s_closeOffset    = prefs.getInt("closeOff", 0);
    s_motorTimeout   = prefs.getInt("motTO", 8000);
    s_pinchThreshold = prefs.getInt("pinch", 300);

    s_autoMode          = prefs.getBool("auto", false);
    s_simulatedHardware = prefs.getBool("simHW", false);
}

// ---------------------------------------------------------
// Save all settings
// ---------------------------------------------------------
void Config_save() {
    prefs.putFloat("lat", s_lat);
    prefs.putFloat("long", s_long);
    prefs.putString("tz", s_timezone);
    prefs.putString("ota", s_otaPassword);

    prefs.putString("bot", s_botToken);
    prefs.putString("chat", s_chatID);

    prefs.putInt("openOff", s_openOffset);
    prefs.putInt("closeOff", s_closeOffset);
    prefs.putInt("motTO", s_motorTimeout);
    prefs.putInt("pinch", s_pinchThreshold);

    prefs.putBool("auto", s_autoMode);
    prefs.putBool("simHW", s_simulatedHardware);
}

// ---------------------------------------------------------
// GETTERS
// ---------------------------------------------------------
float  Config_getLat()            { return s_lat; }
float  Config_getLong()           { return s_long; }
String Config_getTimezone()       { return s_timezone; }
String Config_getOTAPassword()    { return s_otaPassword; }

String Config_getBotToken()       { return s_botToken; }
String Config_getChatID()         { return s_chatID; }

int Config_getOpenOffset()        { return s_openOffset; }
int Config_getCloseOffset()       { return s_closeOffset; }
int Config_getMotorTimeout()      { return s_motorTimeout; }
int Config_getPinchThreshold()    { return s_pinchThreshold; }

bool Config_getAutoMode()         { return s_autoMode; }
bool Config_isSimulatedHardware() { return s_simulatedHardware; }

// ---------------------------------------------------------
// SETTERS
// ---------------------------------------------------------
void Config_setLat(float v)              { s_lat = v; prefs.putFloat("lat", v); }
void Config_setLong(float v)             { s_long = v; prefs.putFloat("long", v); }
void Config_setTimezone(const String& v) { s_timezone = v; prefs.putString("tz", v); }
void Config_setOTAPassword(const String& v) { s_otaPassword = v; prefs.putString("ota", v); }

void Config_setBotToken(const String& v) { s_botToken = v; prefs.putString("bot", v); }
void Config_setChatID(const String& v)   { s_chatID = v; prefs.putString("chat", v); }

void Config_setOpenOffset(int v)         { s_openOffset = v; prefs.putInt("openOff", v); }
void Config_setCloseOffset(int v)        { s_closeOffset = v; prefs.putInt("closeOff", v); }
void Config_setMotorTimeout(int v)       { s_motorTimeout = v; prefs.putInt("motTO", v); }
void Config_setPinchThreshold(int v)     { s_pinchThreshold = v; prefs.putInt("pinch", v); }

void Config_setAutoMode(bool v)          { s_autoMode = v; prefs.putBool("auto", v); }
void Config_setSimulatedHardware(bool v) { s_simulatedHardware = v; prefs.putBool("simHW", v); }
