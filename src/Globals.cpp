#include "Globals.h"

unsigned long bootTime = 0;

extern UniversalTelegramBot bot;

bool autoOpenTriggeredToday = false;
bool autoCloseTriggeredToday = false;

bool lowBatAlertSent = false;

float inputVoltage = 0;
float outputVoltage = 0;

float VOLTAGE_THRESHOLD = 11.5;
float CRITICAL_VOLTAGE = 10.5;

unsigned long totalOpenTime = 0;
unsigned long totalCloseTime = 0;

float avgOpenTime = 0;
float avgCloseTime = 0;

bool firstTimeSyncHandled = false;

bool debugMenuEnabled = false;

bool waitingForOpenOffset = false;
bool waitingForCloseOffset = false;

MenuState menuState = MENU_MAIN;

bool remoteOverride = false;
bool telegramEnabled = true;

int64_t userid = 0;
bool inaOK = false;
float motorCurrent = 0.0f;
bool factoryResetRequested = false;

WiFiClientSecure secured_client;
UniversalTelegramBot bot("", secured_client);
bool testModeActive = false;
