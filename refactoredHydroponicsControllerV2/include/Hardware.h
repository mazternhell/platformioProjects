/*
 * Hardware.h
 * 
 * Hardware I/O functions for controlling pumps, relays, LEDs, 
 * and reading sensors (float switches, touch, encoder).
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include "Globals.h"

// ==================================================
// HARDWARE INITIALIZATION
// ==================================================
void initHardware();

// ==================================================
// INPUT UPDATES (Non-blocking)
// ==================================================
void updateFloatSwitches(unsigned long currentTime);
void updateTouchSensors(unsigned long currentTime);
void updateEncoder();

// ==================================================
// ENCODER ISR
// ==================================================
void IRAM_ATTR encoderISR();

// ==================================================
// OUTPUT CONTROL
// ==================================================
void setRelay(uint8_t relay, bool state);
void setPumpSpeed(uint8_t pump, uint8_t speed);
void setLED(uint8_t led, bool state);
void setWS2812B(uint8_t r, uint8_t g, uint8_t b);

// ==================================================
// INPUT READING
// ==================================================
uint8_t getPumpSpeed(uint8_t pump);
bool getFloatSwitch(uint8_t level);

// ==================================================
// JSON EXPORT
// ==================================================
String getHardwareJSON();

#endif // HARDWARE_H
