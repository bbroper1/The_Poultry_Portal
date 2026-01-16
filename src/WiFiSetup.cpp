#include "WiFiSetup.h"
#include <WiFiManager.h>
#include <Arduino.h>

bool WiFiSetup_begin() {
    WiFiManager wm;

    wm.setConnectTimeout(20);
    wm.setConfigPortalTimeout(180);   // optional: auto-close AP after 3 minutes

    if (!wm.autoConnect("PoultryPortal_AP")) {
        Serial.println("❌ WiFi connection failed");
        return false;
    }

    Serial.println("✅ WiFi connected");
    return true;
}
