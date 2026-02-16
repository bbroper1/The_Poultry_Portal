#pragma once
#include "../../telegram/TelegramEvent.h"
#include "../../telegram/TelegramClient.h"

namespace OffsetHandler {

void handle(const TelegramEvent& evt, TelegramClient* client);

// Internal state
extern bool waitingForOpen;
extern bool waitingForClose;

}