#pragma once

#include "../../telegram/TelegramEvent.h"
#include "../../telegram/TelegramClient.h"

namespace DebugHandler {

void handle(const TelegramEvent& evt, TelegramClient* client);

} // namespace DebugHandler