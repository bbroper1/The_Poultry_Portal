#include "TelegramHandler.h"
#include "TelegramClient.h"

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

// Config (needed for location)
#include "modules/config/Config.h"

#include <Arduino.h>

namespace TelegramHandler {

void handle(const TelegramEvent& evt, TelegramClient* client) {

    Serial.println(">>> HANDLER: dispatching event type = " + String(evt.type));

    switch (evt.type) {

        // -------------------------------------------------
        // CORE COMMANDS
        // -------------------------------------------------
        case EVT_STATUS:
            StatusHandler::handle(evt, client);
            return;

        case EVT_HEALTH:
            HealthHandler::handle(evt, client);
            return;

        case EVT_ENERGY:
            EnergyHandler::handle(evt, client);
            return;

        case EVT_LOGS:
            LogsHandler::handle(evt, client);
            return;

        case EVT_HELP:
            client->sendMessageWithKeyboard(
                evt.chatId,
                "🆘 *Help & Commands*\n"
                "━━━━━━━━━━━━━━━━━━\n\n"

                "Here’s everything your PoultryPortal can do:\n\n"

                "📊 *Status & System Info*\n"
                "• /status – Full system status\n"
                "• /health – Motor health & diagnostics\n"
                "• /energy – Power usage & current draw\n"
                "• /logs – Recent activity (with pages)\n\n"

                "⚙️ *Settings & Configuration*\n"
                "• /settings – Open settings menu\n"
                "• /timezone – Set your local timezone\n"
                "• /location – Update GPS location\n"
                "• Open/Close offsets & motor settings live in menus\n\n"

                "🚪 *Door Control*\n"
                "• /open – Open the door\n"
                "• /close – Close the door\n"
                "• /auto – Enable automatic sunrise/sunset mode\n\n"

                "🛠 *Debug Tools*\n"
                "• /debug – Debug menu (door, time, sun, limits, energy)\n"
                "• /sim – Simulation mode controls\n\n"

                "📌 *Tips*\n"
                "• Use the on‑screen buttons for quick navigation\n"
                "• Logs support *Next*, *Prev*, *First*, *Last* pages\n"
                "• Settings and Debug menus include BACK buttons\n"
                "• Location pin updates sunrise/sunset calculations\n\n"

                "If you ever get lost, just press *BACK* or return to the main menu.",
                kbMain()
            );
            return;

        // -------------------------------------------------
        // MENU NAVIGATION
        // -------------------------------------------------
        case EVT_SHOW_MAIN_MENU:
            MenuHandler::handle(evt, client);
            return;

        case EVT_SHOW_SETTINGS:
            SettingsHandler::handle(evt, client);
            return;

        case EVT_SHOW_DEBUG:
            DebugHandler::handle(evt, client);
            return;

        case EVT_SHOW_TIMEZONE:
        case EVT_SET_TIMEZONE:
        case EVT_TIMEZONE_OTHER_EUROPE:
            TimezoneHandler::handle(evt, client);
            return;

        case EVT_SHOW_MOTOR_MENU:
            MotorHandler::handle(evt, client);
            return;

        // -------------------------------------------------
        // DOOR CONTROL
        // -------------------------------------------------
        case EVT_OPEN:
            DoorController::openDoor();
            client->sendMessageWithKeyboard(evt.chatId, "👐 Opening door…", kbMain());
            return;

        case EVT_CLOSE:
            DoorController::closeDoor();
            client->sendMessageWithKeyboard(evt.chatId, "🚪 Closing door…", kbMain());
            return;

        case EVT_AUTO:
            DoorController::enableAutoMode(true);
            client->sendMessageWithKeyboard(evt.chatId, "🤖 Auto mode enabled", kbMain());
            return;

        // -------------------------------------------------
        // LOCATION UPDATE
        // -------------------------------------------------
        case EVT_LOCATION: {
            Config_setLat(evt.latitude);
            Config_setLong(evt.longitude);
            Config_save();

            client->sendMessageWithKeyboard(
                evt.chatId,
                "📍 Location updated:\n"
                "Lat: " + String(evt.latitude, 6) + "\n"
                "Lon: " + String(evt.longitude, 6),
                kbMain()
            );
            return;
        }

        // -------------------------------------------------
        // OFFSET HANDLING
        // -------------------------------------------------
        case EVT_SET_OPEN_OFFSET:
        case EVT_SET_CLOSE_OFFSET:
        case EVT_OFFSET_VALUE:
            OffsetHandler::handle(evt, client);
            return;

        // -------------------------------------------------
        // MOTOR HANDLING
        // -------------------------------------------------
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

        // -------------------------------------------------
        // DEBUG HANDLING
        // -------------------------------------------------
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

        // -------------------------------------------------

        // -------------------------------------------------
        // SIMULATION MODE COMMANDS
        // -------------------------------------------------
        case EVT_SIM_ON:
        case EVT_SIM_OFF:
        case EVT_SIM_STATUS:
        case EVT_SIM_TEST:
        case EVT_SIM_JAM:
        case EVT_SET_TOKEN:
        case EVT_SET_CHAT:
            SimHandler::handle(evt, client);
            return;

        // -------------------------------------------------
        // LOGS HANDLING
        // -------------------------------------------------
        case EVT_SHOW_LOGS:
        case EVT_LOGS_NEXT:
        case EVT_LOGS_PREV:
        case EVT_LOGS_FIRST:
        case EVT_LOGS_LAST:
            LogsHandler::handle(evt, client);
            return;

        // -------------------------------------------------
        // UNIVERSAL BACK HANDLING
        // -------------------------------------------------
        case EVT_BACK:
            switch (menuState) {

                case MENU_TIMEZONE:
                    SettingsHandler::handle(evt, client);
                    return;

                case MENU_SETTINGS:
                    MenuHandler::handle(evt, client);
                    return;

                case MENU_DEBUG:
                    SettingsHandler::handle(evt, client);
                    return;

                case MENU_MOTOR:
                    SettingsHandler::handle(evt, client);
                    return;

                default:
                    MenuHandler::handle(evt, client);
                    return;
            }

        // -------------------------------------------------
        // UNKNOWN / UNMIGRATED
        // -------------------------------------------------
        default:
            client->sendMessageWithKeyboard(
                evt.chatId,
                "❓ I didn’t understand that command.",
                kbMain()
            );
            return;
    }
}

} // namespace TelegramHandler