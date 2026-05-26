#include "TelegramHandler.h"
#include "TelegramClient.h"

// Keyboards
#include "modules/keyboards/TelegramKeyboards.h"

// Modular handlers
#include "handlers/StatusHandler.h"
#include "handlers/HealthHandler.h"
#include "handlers/EnergyHandler.h"
#include "handlers/LogsHandler.h"
#include "handlers/MenuHandler.h"
#include "handlers/SettingsHandler.h"
#include "handlers/DebugHandler.h"
#include "handlers/TimezoneHandler.h"
#include "handlers/SimHandler.h"
#include "handlers/ConfigHandler.h"
#include "handlers/OffsetHandler.h"
#include "handlers/MotorHandler.h"

// Door control
#include "modules/door/DoorController.h"

// Config
#include "modules/config/Config.h"

// Globals
#include "modules/system/Globals.h"

// Time utilities
#include "modules/utils/TimeUtils.h"
#include "modules/time/TimeManager.h"
#include "modules/scheduler/SunContext.h"

#include <Arduino.h>

extern SunSet sun;

// ---------------------------------------------------------
// Formatting Helper
// ---------------------------------------------------------
String TelegramHandler::blockHeader(const String& emoji, const String& title) {
    return emoji + " *" + title + "*\n━━━━━━━━━━━━━━━\n";
}

// ---------------------------------------------------------
// MAIN DISPATCHER
// ---------------------------------------------------------
void TelegramHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    Serial.println(">>> HANDLER: dispatching event type = " + String(evt.type));

    // ---------------------------------------------------------
    // OFFSET VALUE ENTRY (text → EVT_OFFSET_VALUE)
    // ---------------------------------------------------------
    if ((OffsetHandler::isWaitingForOpen() || OffsetHandler::isWaitingForClose()) &&
        evt.type == EVT_MOTOR_VALUE)
    {
        TelegramEvent e = evt;
        e.type = EVT_OFFSET_VALUE;
        OffsetHandler::handle(e, client);
        return;
    }

    switch (evt.type) {

        // ---------------------------------------------------------
        // CORE COMMANDS
        // ---------------------------------------------------------
        case EVT_STATUS:        StatusHandler::handle(evt, client); return;
        case EVT_HEALTH:        HealthHandler::handle(evt, client); return;
        case EVT_ENERGY:        EnergyHandler::handle(evt, client); return;
        case EVT_LOGS:          LogsHandler::handle(evt, client); return;

        case EVT_HELP:
            client->sendMessageWithKeyboard(
                evt.chatId,
                blockHeader("🆘", "HELP & COMMANDS") +
                "Use the on‑screen buttons for quick navigation.\n"
                "If you ever get lost, press BACK.",
                kbMain()
            );
            return;

        // ---------------------------------------------------------
        // MENU NAVIGATION
        // ---------------------------------------------------------
        case EVT_SHOW_MAIN_MENU:    MenuHandler::handle(evt, client); return;
        case EVT_SHOW_SETTINGS:     SettingsHandler::handle(evt, client); return;
        case EVT_SHOW_DEBUG:        DebugHandler::handle(evt, client); return;

        case EVT_SHOW_TIMEZONE:
        case EVT_SET_TIMEZONE:
        case EVT_TIMEZONE_OTHER_EUROPE:
            TimezoneHandler::handle(evt, client);
            return;

        case EVT_SHOW_MOTOR_MENU:
            MotorHandler::handle(evt, client);
            return;

        // ---------------------------------------------------------
        // DOOR CONTROL — SHOW OVERRIDE MENU
        // ---------------------------------------------------------
        case EVT_OPEN:
            overrideIsOpenCommand = true;
            client->sendMessageWithKeyboard(
                evt.chatId,
                blockHeader("⏳", "MANUAL OVERRIDE") +
                "How long should I keep the door OPEN?",
                kbOverrideMenu(true)
            );
            return;

        case EVT_CLOSE:
            overrideIsOpenCommand = false;
            client->sendMessageWithKeyboard(
                evt.chatId,
                blockHeader("⏳", "MANUAL OVERRIDE") +
                "How long should I keep the door CLOSED?",
                kbOverrideMenu(false)
            );
            return;

        case EVT_AUTO:
            DoorController::enableAutoMode(true);
            remoteOverride = false;
            client->sendMessageWithKeyboard(evt.chatId, "🤖 Auto mode enabled", kbMain());
            return;

        // ---------------------------------------------------------
        // OVERRIDE DURATION HANDLING
        // ---------------------------------------------------------
        case EVT_OVERRIDE_15:
        case EVT_OVERRIDE_30:
        case EVT_OVERRIDE_60:
        case EVT_OVERRIDE_SUNSET:
        case EVT_OVERRIDE_SUNRISE:
        {
            time_t now = time(nullptr);

            switch (evt.type) {
                case EVT_OVERRIDE_15:      remoteOverrideUntil = now + 15 * 60; break;
                case EVT_OVERRIDE_30:      remoteOverrideUntil = now + 30 * 60; break;
                case EVT_OVERRIDE_60:      remoteOverrideUntil = now + 60 * 60; break;

                case EVT_OVERRIDE_SUNSET: {
                    struct tm nowTm = TimeManager::getLocalTime();
                    sun.setCurrentDate(nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday);
                    sun.setPosition(Config_getLat(), Config_getLong(), TimeManager::utcOffsetHours());
                    int sunsetMin = (int)sun.calcSunset() + Config_getCloseOffset();
                    remoteOverrideUntil = TimeUtils::todayAtMinutes(sunsetMin);
                    break;
                }

                case EVT_OVERRIDE_SUNRISE: {
                    struct tm nowTm = TimeManager::getLocalTime();
                    sun.setCurrentDate(nowTm.tm_year + 1900, nowTm.tm_mon + 1, nowTm.tm_mday + 1);
                    sun.setPosition(Config_getLat(), Config_getLong(), TimeManager::utcOffsetHours());
                    int sunriseMin = (int)sun.calcSunrise() + Config_getOpenOffset();
                    remoteOverrideUntil = TimeUtils::tomorrowAtMinutes(sunriseMin);
                    break;
                }
            }

            remoteOverride = true;
            String untilStr = TimeUtils::formatTimestamp(remoteOverrideUntil);

            if (overrideIsOpenCommand) {
                DoorController::openDoor();
                client->sendMessageWithKeyboard(
                    evt.chatId,
                    "👐 Door opened.\n⏳ Override active until: " + untilStr,
                    kbMain()
                );
            } else {
                DoorController::closeDoor();
                client->sendMessageWithKeyboard(
                    evt.chatId,
                    "🚪 Door closed.\n⏳ Override active until: " + untilStr,
                    kbMain()
                );
            }
            return;
        }

        case EVT_OVERRIDE_CANCEL:
            remoteOverride = false;
            client->sendMessageWithKeyboard(
                evt.chatId,
                "❌ Manual override cancelled.\n🤖 Auto mode resumed.",
                kbMain()
            );
            return;

        // ---------------------------------------------------------
        // LOCATION UPDATE
        // ---------------------------------------------------------
        case EVT_LOCATION:
            Config_setLat(evt.latitude);
            Config_setLong(evt.longitude);
            Config_save();

            client->sendMessageWithKeyboard(
                evt.chatId,
                blockHeader("📍", "LOCATION UPDATED") +
                "Lat: " + String(evt.latitude, 6) + "\n" +
                "Lon: " + String(evt.longitude, 6),
                kbMain()
            );
            return;

        // ---------------------------------------------------------
        // OFFSET HANDLING
        // ---------------------------------------------------------
        case EVT_SET_OPEN_OFFSET:
        case EVT_SET_CLOSE_OFFSET:
        case EVT_OFFSET_VALUE:
            OffsetHandler::handle(evt, client);
            return;

        // ---------------------------------------------------------
        // MOTOR HANDLING
        // ---------------------------------------------------------
        case EVT_SET_MOTOR_TIMEOUT:
        case EVT_SET_PINCH_THRESHOLD:
        case EVT_MOTOR_VALUE:
        case EVT_MOTOR_STATUS:
        case EVT_MOTOR_TEST:
        case EVT_MOTOR_STOP:
        case EVT_MOTOR_RESET_HEALTH:
        case EVT_MOTOR_FORCE_STUCK:
            MotorHandler::handle(evt, client);
            return;

        // ---------------------------------------------------------
        // DEBUG HANDLING
        // ---------------------------------------------------------
        case EVT_DEBUG_DOOR:
        case EVT_DEBUG_TIME:
        case EVT_DEBUG_SUN:
        case EVT_DEBUG_AUTO:
        case EVT_DEBUG_STATE:
        case EVT_DEBUG_LIMITS:
        case EVT_DEBUG_ENERGY:
        case EVT_DEBUG_ALL:
        case EVT_DEBUG_CONFIG:
        case EVT_DEBUG_ON:
        case EVT_DEBUG_OFF:
            DebugHandler::handle(evt, client);
            return;

        // ---------------------------------------------------------
        // SIMULATION MODE COMMANDS
        // ---------------------------------------------------------
        case EVT_SIM_ON:
        case EVT_SIM_OFF:
        case EVT_SIM_STATUS:
        case EVT_SIM_TEST:
        case EVT_SIM_JAM:
        case EVT_SET_TOKEN:
        case EVT_SET_CHAT:
            SimHandler::handle(evt, client);
            return;

        // ---------------------------------------------------------
        // LOGS HANDLING
        // ---------------------------------------------------------
        case EVT_SHOW_LOGS:
        case EVT_LOGS_NEXT:
        case EVT_LOGS_PREV:
        case EVT_LOGS_FIRST:
        case EVT_LOGS_LAST:
            LogsHandler::handle(evt, client);
            return;

        // ---------------------------------------------------------
        // UNIVERSAL BACK HANDLING
        // ---------------------------------------------------------
        case EVT_BACK:
            switch (menuState) {
                case MENU_TIMEZONE: SettingsHandler::handle(evt, client); return;
                case MENU_SETTINGS: MenuHandler::handle(evt, client); return;
                case MENU_DEBUG:    SettingsHandler::handle(evt, client); return;
                case MENU_MOTOR:    SettingsHandler::handle(evt, client); return;
                default:            MenuHandler::handle(evt, client); return;
            }

        // ---------------------------------------------------------
        // UNKNOWN
        // ---------------------------------------------------------
        default:
            client->sendMessageWithKeyboard(
                evt.chatId,
                blockHeader("❓", "UNKNOWN COMMAND") +
                "I didn’t understand that command.",
                kbMain()
            );
            return;
    }
}
