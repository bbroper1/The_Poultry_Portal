#pragma once
#include <AsyncTelegram2.h>
#include "Globals.h"

void TelegramRouter_init();
void TelegramRouter_handle();

ReplyKeyboard buildMainKeyboard();
ReplyKeyboard buildSettingsKeyboard();
ReplyKeyboard buildDebugKeyboard();

