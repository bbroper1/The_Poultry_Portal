#include "EnergyHandler.h"
#include "../../keyboards/TelegramKeyboards.h"

// System modules
#include "EnergyModule.h"
#include "BatteryModule.h"
#include "SystemStatus.h"
#include "modules/time/TimeManager.h"
#include "Config.h"

void EnergyHandler::handle(const TelegramEvent& evt, TelegramClient* client) {

    String kbd = kbMain();

    // ---------------------------------------------------------
    // Build 30-day data series
    // ---------------------------------------------------------
    // Your old code repeated the same value 30 times.
    // We preserve that behavior exactly.
    String dp = "";
    for (int i = 29; i >= 0; i--) {
        dp += String(Energy_getMonthlymAh() / 30.0, 0);
        if (i > 0) dp += ",";
    }

    // 7-day average (legacy behavior: same as dp)
    String avg = dp;

    // ---------------------------------------------------------
    // Build QuickChart JSON
    // ---------------------------------------------------------
    String json = "{"
      "type:'line',"
      "data:{"
        "labels:[30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1],"
        "datasets:["
          "{label:'Daily mAh',data:[" + dp + "],fill:true,"
          "backgroundColor:'rgba(54,162,235,0.2)',"
          "borderColor:'rgb(54,162,235)',"
          "borderWidth:2},"
          "{label:'7-Day Avg',data:[" + avg + "],fill:false,"
          "borderColor:'rgb(255,99,132)',"
          "borderWidth:2,"
          "tension:0.3}"
        "]"
      "},"
      "options:{"
        "plugins:{"
          "title:{display:true,text:'30-Day Energy Usage',font:{size:18}}"
        "},"
        "scales:{"
          "x:{grid:{display:true}},"
          "y:{grid:{display:true}}"
        "}"
      "}"
    "}";

    json.replace(" ", "");

    // QuickChart PNG URL
    String chartPng = "https://quickchart.io/chart.png?chart=" + json;

    // ---------------------------------------------------------
    // Send chart as photo
    // ---------------------------------------------------------
    client->sendPhotoByUrl(evt.chatId, chartPng, "");

    // (Optional) If you want a caption or summary, we can add it later.
}