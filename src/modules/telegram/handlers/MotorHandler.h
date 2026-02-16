#pragma once
#include "../../telegram/TelegramEvent.h"
#include "../../telegram/TelegramClient.h"

namespace MotorHandler {

extern bool waitingForTimeout;
extern bool waitingForPinch;

void handle(const TelegramEvent& evt, TelegramClient* client);

} // namespace MotorHandler