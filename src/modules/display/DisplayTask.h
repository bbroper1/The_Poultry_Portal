#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>

// Start the FreeRTOS display task
void DisplayTask_begin();

// Power control
void Display_wake();
void Display_sleep();

// Legacy compatibility (kept for drop‑in)
void Display_init();     // no-op
void Display_showBoot(); // no-op
void Display_update();   // no-op