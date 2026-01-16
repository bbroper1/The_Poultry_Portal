#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>


// Public API
void Display_init();
void Display_wake();
void Display_sleep();
void Display_showBoot();
void Display_update();

// Optional direct screen calls (if needed)
void Display_showMain();
void Display_showPower();

// Expose the display object if other modules need it
extern Adafruit_SSD1306 display;
void Display_begin();

