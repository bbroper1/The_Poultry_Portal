#include "LogsHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

#include "Logging.h"
#include "modules/utils/TimeUtils.h"

static const int LOGS_PER_PAGE = 10;

void LogsHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    int total = getLogCount();
    if (total == 0) {
        client->sendMessageWithKeyboard(
            evt.chatId,
            "📋 *RECENT ACTIVITY*\n━━━━━━━━━━━━━━━\nNo logs yet.",
            kbMain()
        );
        return;
    }

    // Persistent page index
    static int currentPage = 0;

    // Determine action
    switch (evt.type) {
        case EVT_SHOW_LOGS:
            currentPage = 0;
            break;

        case EVT_LOGS_NEXT:
            currentPage++;
            break;

        case EVT_LOGS_PREV:
            currentPage--;
            break;

        case EVT_LOGS_FIRST:
            currentPage = 0;
            break;

        case EVT_LOGS_LAST:
            currentPage = (total - 1) / LOGS_PER_PAGE;
            break;

        default:
            break;
    }

    // Clamp page
    int maxPage = (total - 1) / LOGS_PER_PAGE;
    if (currentPage < 0) currentPage = 0;
    if (currentPage > maxPage) currentPage = maxPage;

    // Build output
    String out;
    out.reserve(2000);

    out += "📋 *RECENT ACTIVITY*\n";
    out += "━━━━━━━━━━━━━━━\n";
    out += "Page " + String(currentPage + 1) + " of " + String(maxPage + 1) + "\n\n";

    // Newest → oldest
    int start = total - 1 - (currentPage * LOGS_PER_PAGE);
    int end   = max(start - LOGS_PER_PAGE + 1, 0);

    for (int i = start; i >= end; i--) {
        LogEntry e = getLog(i);
        if (e.message.length() == 0)
            continue;

        out += "• ";
        out += Log_formatEntry(e);
        out += "\n";
    }

    client->sendMessageWithKeyboard(
        evt.chatId,
        out,
        kbLogs(currentPage, maxPage)
    );
}