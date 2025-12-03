/*
 * ============================================
 * ESP32-S3 Hydroponics Controller
 * ============================================
 * 
 * Features:
 * - Dual-core architecture (Display on Core 1, Background tasks on Core 0)
 * - 128x128 TFT display with rotary encoder navigation
 * - 4x Peristaltic pump control (dosing schedules)
 * - 8x Relay outlets (on/off schedules)  
 * - WiFi + MQTT connectivity with async reconnection
 * - LittleFS file storage for schedules and config
 * - Web interface for monitoring and control
 * - NTP time synchronization
 * - Float switch and touch sensor support
 * 
 * Hardware:
 * - ESP32-S3-DevKitC-1
 * - GC9A01 128x128 Round TFT Display
 * - Rotary Encoder (CLK, DT, SW)
 * - 4x Peristaltic Pumps
 * - 8x Relay Module
 * 
 * Author: [Your Name]
 * Version: 2.0
 * Date: 2025-01-04
 * 
 * ============================================
 */

/*
 * Hydroponics Controller - OPTIMIZED VERSION
 * ESP32-S3 DevKit C
 *
 * OPTIMIZATIONS APPLIED:
 * - PROGMEM for menu strings (saves ~60KB RAM)
 * - Conditional debug output (saves ~80KB flash)
 * - Encoder button polling (more reliable)
 * - String deduplication (saves ~10KB)
 * - Optimized JSON responses (saves ~15KB)
 *
 * Flash savings: ~280KB (from 81% to ~62%)
 *
 * DEBUG_LEVEL: Set in platformio.ini or below
 * 0 = No debug output
 * 1 = Errors only
 * 2 = Errors + Info
 * 3 = Verbose (all debug)
 */

// ============================================
// USB SERIAL CONFIGURATION (for CH343 or Native USB)
// ============================================
// Uncomment the lines below to use ESP32-S3 native USB instead of CH343:
//#ifndef ARDUINO_USB_MODE
//#define ARDUINO_USB_MODE 1
//#endif
//#ifndef ARDUINO_USB_CDC_ON_BOOT  
//#define ARDUINO_USB_CDC_ON_BOOT 1
//#endif

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0  // Default: errors only
#endif

#define PMSTR(s) (__FlashStringHelper*)(s)  // Helper for PROGMEM strings

#include "Globals.h"
#include "Storage.h"
#include "Hardware.h"
#include "DisplayUI.h"
#include "WebPages.h"
#include "WebServer.h"
#include "SimpleWiFi.h"
#include "Tasks.h"
#include "MenuRegistry.h"
#include <Arduino.h>
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
// WiFiManager removed - using SimpleWiFi module instead
#include <PubSubClient.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <RTClib.h>
#include <time.h>
#include <TFT_ILI9163C.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>


// AsyncWebServer objects (kept in main.cpp to avoid library conflicts)
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//restart ESP32 after WiFi connects/reconnects
volatile bool pendingRestart = false;
unsigned long restartAt = 0;

void scheduleRestart(unsigned long delayMs) {
  pendingRestart = true;
  restartAt = millis() + delayMs;
}

// ==================================================
// ENCODER ISR - MUST BE IN MAIN.CPP FOR PROPER LINKAGE
// ==================================================
void IRAM_ATTR encoderISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderTime < ENCODER_DEBOUNCE_MS) {
    return;
  }

  int MSB = digitalRead(ENCODER_DT);
  int LSB = digitalRead(ENCODER_CLK);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    pulseCounter--;
    if (pulseCounter <= -PULSES_PER_STEP) {
      encoderPosition--;
      pulseCounter = 0;
      lastEncoderTime = currentTime;
    }
  }
  else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    pulseCounter++;
    if (pulseCounter >= PULSES_PER_STEP) {
      encoderPosition++;
      pulseCounter = 0;
      lastEncoderTime = currentTime;
    }
  }

  lastEncoded = encoded;
}

// ============================================
// SUBSYSTEM API TABLE — FORWARD DECLARATIONS
// ============================================

// --- STORAGE / CONFIG ---
bool initLittleFS();
void setDefaultConfig();
bool loadConfigFromLittleFS();
bool saveConfigToLittleFS();
void loadSchedulesFromStorage();
void saveSchedulesToStorage();
void loadPumpCalibrationsFromStorage();
void savePumpCalibrationsToStorage();
void loadTopUpConfigFromStorage();
void saveTopUpConfigToStorage();
void loadReplaceConfigFromStorage();
void saveReplaceConfigToStorage();

// --- WEB SERVER / JSON ---
void setupWebServer();
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void notifyWebClients();
void updateSensorData();
String getSensorDataJSON();
String getConfigJSON();
bool updateConfigFromJSON(String json);
String getIndexHTML();
String getHardwareJSON();

// --- NETWORK (WIFI/MQTT/NTP) ---
void handleWiFiState(unsigned long currentTime);
void initOnlineServices();
void startNTPSync();
void updateNTPSync();
void resetNTPSync();
void checkDailySync(unsigned long currentTime);
void handleMQTT();
void connectMQTT();
void reconnectMQTT(unsigned long currentTime);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishSensorData(unsigned long currentTime);

// --- HARDWARE / IO ---
void IRAM_ATTR encoderISR();
void initHardware();
void updateEncoder();
void updateFloatSwitches(unsigned long currentTime);
void updateTouchSensors(unsigned long currentTime);
void setRelay(uint8_t relay, bool state);
void setPumpSpeed(uint8_t pump, uint8_t speed);
uint8_t getPumpSpeed(uint8_t pump);
bool getFloatSwitch(uint8_t level);
void setLED(uint8_t led, bool state);
void setWS2812B(uint8_t r, uint8_t g, uint8_t b);

// --- SCHEDULE EXECUTION ---
void checkDosingSchedules(unsigned long currentTime);
void startDosing(uint8_t scheduleIndex, unsigned long currentTime);
void updateDosingExecution(unsigned long currentTime);

// --- MENU SYSTEM ---
void handleMenuNavigation();
void drawMenu();
void drawMainMenu();
void drawScheduleEditorScreen();
void drawDosingScheduleListScreen();
void drawDosingDeleteListScreen();
void drawDaySelectionScreen();
void drawTimeSelectionScreen();
void drawAmountSelectionScreen();
void drawConfirmDialog(const char* title);
void drawCalibrateMenu(uint8_t pumpNum);
void handleMenuSelection();
void resetToMainScreen();
void handleDosingDeleteMenu();
bool checkMenuTimeout(unsigned long currentTime);
void drawGenericMenu(const char* title, const char* const* items, int itemCount, bool useScrolling);

// --- DISPLAY / UI ---
void initDisplay();
void updateDisplay();
void updateWifiStatus(const char* status);
void updateMqttStatus(const char* status);
void updateNtpStatus(const char* status);
void drawXIcon(int x, int y);
void drawCircleIndicator(int x, int y, bool connected);
void drawStatusBar();
void updateStatusBar();

// --- FREERTOS TASKS ---
void SensorTask(void *parameter);

// --- MISC ---
void logSensorData();

// Check if schedule should run today based on day bitmap
bool isScheduleActiveToday(uint8_t daysBitmap) {
  DateTime now = rtc.now();
  uint8_t todayIndex = now.dayOfTheWeek(); // 0=Sun, 1=Mon, etc.
  return isDayEnabled(daysBitmap, todayIndex);
}

// ============================================
// MENU SYSTEM IMPLEMENTATION
// ============================================

// Initialize menu system and load saved data
void initMenuSystem() {

  // Initialize all schedules to disabled
  for (int i = 0; i < MAX_DOSING_SCHEDULES; i++) {
    dosingSchedules[i].enabled = false;
  }
  for (int i = 0; i < MAX_OUTLET_SCHEDULES; i++) {
    outletSchedules[i].enabled = false;
  }

  // Initialize pump calibrations to defaults
  for (int i = 0; i < 4; i++) {
    pumpCalibrations[i].pwmSpeed = 50;
    pumpCalibrations[i].timeMs = 1000;
    pumpCalibrations[i].mlPerSecond = 1.0;
  }

  // Initialize configs
  topUpConfig.pump1ML = 0;
  topUpConfig.pump2ML = 0;
  topUpConfig.pump3ML = 0;
  topUpConfig.pump4ML = 0;
  topUpConfig.fillPumpRelay = 1;

  replaceConfig.pump1ML = 0;
  replaceConfig.pump2ML = 0;
  replaceConfig.pump3ML = 0;
  replaceConfig.pump4ML = 0;
  replaceConfig.drainRelay = 1;
  replaceConfig.fillRelay = 2;

  // Load from storage
  loadSchedulesFromStorage();
  loadPumpCalibrationsFromStorage();
  loadTopUpConfigFromStorage();
  loadReplaceConfigFromStorage();

}

// Check if menu has timed out
bool checkMenuTimeout(unsigned long currentTime) {
  if (menuNav.currentMenu != MENU_MAIN &&
      (currentTime - menuNav.lastActivity) > MENU_TIMEOUT) {
    resetToMainScreen();
    return true;
  }
  return false;
}

// Reset to main screen
void resetToMainScreen() {
  menuNav.currentMenu = MENU_MAIN;
  menuNav.selectedIndex = 0;
  menuNav.scrollOffset = 0;
  menuNav.inEditMode = false;
  menuNav.editingHour = true;
  menuNav.daySelectIndex = 0;
  menuNav.needsRedraw = true;
  menuNav.needsFullRedraw = true;
  menuNav.lastDrawnIndex = -1;
  menuNav.tempIndex = -1;

  noInterrupts();
  encoderPosition = 0;   // reset the real volatile encoder
  interrupts();
  
  menuNav.lastActivity = millis();
}

// Generic helper to navigate to any menu
void navigateToMenu(MenuState menu, int selectedIndex = 0) {
  menuNav.currentMenu = menu;
  menuNav.selectedIndex = selectedIndex;
  menuNav.scrollOffset = 0;
  menuNav.needsRedraw = true;
  menuNav.needsFullRedraw = true;
  menuNav.lastDrawnIndex = -1;
  menuNav.lastActivity = millis();
  
  // RESET ENCODER ATOMICALLY - prevents encoder position carryover between menus
  noInterrupts();
  encoderPosition = selectedIndex;  // Reset volatile directly with ISR protection
  interrupts();
}

// Forward declarations for menu handlers
void handleMainMenu();
void handleSchedulingMenu();
void handleDosingScheduleMenu();
void handleDosingViewMenu();
void handleDosingAddMenu();
void handleDaySelectionMenu();
void handleManualDosingMenu();
void handleOutletScheduleMenu();
void handleOutletViewMenu();
void handlePumpCalibrationMenu();
void handleCalibrateMenu();
void handleTopUpMenu();
void handleReplaceSolutionMenu();
void handleConfirmMenu();
void handleOutletAddMenu();
void selectOutletAddMenu();
void selectDosingViewMenu();
void selectOutletViewMenu();
void handleOutletDeleteMenu();
void selectOutletDeleteSelectMenu();
void selectOutletDeleteConfirmMenu();
void selectDosingErrorNoDaysMenu();



// Generic encoder bounds checker - prevents overflow/underflow
void clampEncoderPosition(int minVal, int maxVal) {
  // Read and clamp encoderPosition directly (no hardware.encoderPosition copy)
  noInterrupts();
  if (encoderPosition < 0) encoderPosition = 0;
  if (encoderPosition > maxVal) encoderPosition = maxVal;
  menuNav.selectedIndex = encoderPosition;
  interrupts();
}

// Default navigation handler for generic menus
static void defaultMenuNav(const MenuDef& m) {
  if (m.itemCount <= 0) {
    clampEncoderPosition(0, 0);
    menuNav.scrollOffset = 0;
    return;
  }

  int maxIndex = m.itemCount - 1;
  clampEncoderPosition(0, maxIndex);

  if (m.useScrolling) {
    if (menuNav.selectedIndex < menuNav.scrollOffset)
      menuNav.scrollOffset = menuNav.selectedIndex;
    if (menuNav.selectedIndex >= menuNav.scrollOffset + MENU_ITEMS_PER_PAGE)
      menuNav.scrollOffset = menuNav.selectedIndex - MENU_ITEMS_PER_PAGE + 1;
  } else {
    menuNav.scrollOffset = 0;
  }
}


// Legacy selection switch (we'll keep it for menus not migrated)
static void handleMenuSelectionLegacy();

// New master navigation dispatcher
void handleMenuNavigation() {
  // Preserve your activity + redraw trigger
  static int lastEncoderPos = 0;
  if (encoderPosition != lastEncoderPos) {
    menuNav.lastActivity = millis();
    lastEncoderPos = encoderPosition;
    menuNav.needsRedraw = true;
  }

  const MenuDef& m = MENUS[menuNav.currentMenu];
  if (m.navFn) m.navFn();
  else defaultMenuNav(m);

    // ✅ RESTORE BUTTON SELECT HANDLING (this was lost in refactor)
  if (hardware.encoderButton) {
    hardware.encoderButton = false;
    menuNav.lastActivity = millis();
    handleMenuSelection();
    menuNav.needsFullRedraw = true;
  }
}

// ========== INDIVIDUAL MENU HANDLERS ==========

// Main Menu Handler - TESTING DIRECT encoderPosition ACCESS
void handleMainMenu() {
  const int maxIndex = mainMenuCount - 1;  // 0-6
  clampEncoderPosition(0, maxIndex);
  // Read and clamp encoderPosition directly (no hardware.encoderPosition copy)
  // noInterrupts();
  // if (encoderPosition < 0) encoderPosition = 0;
  // if (encoderPosition > maxIndex) encoderPosition = maxIndex;
  // menuNav.selectedIndex = encoderPosition;
  // interrupts();
  
  // Handle scrolling
  if (menuNav.selectedIndex < menuNav.scrollOffset) {
    menuNav.scrollOffset = menuNav.selectedIndex;
    menuNav.needsFullRedraw = true;
  } else if (menuNav.selectedIndex >= menuNav.scrollOffset + MENU_ITEMS_PER_PAGE) {
    menuNav.scrollOffset = menuNav.selectedIndex - MENU_ITEMS_PER_PAGE + 1;
    menuNav.needsFullRedraw = true;
  }
}

// Scheduling Menu Handler (parent menu)
void handleSchedulingMenu() {
  const int maxIndex = schedulingMenuCount - 1;  // 0-2
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Dosing Schedule Menu Handler
void handleDosingScheduleMenu() {
  const int maxIndex = dosingScheduleMenuCount - 1;  // 0-4
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Dosing View Menu Handler (page-based navigation)
void handleDosingViewMenu() {
  // Calculate total pages (6 schedules per page, max 4 pages)
  const int SCHEDULES_PER_PAGE = 6;
  int totalPages = (dosingScheduleCount + SCHEDULES_PER_PAGE - 1) / SCHEDULES_PER_PAGE;
  if (totalPages > 4) totalPages = 4;  // Limit to 4 pages max
  if (totalPages == 0) totalPages = 1;  // At least 1 page (for "no schedules" message)
  
  const int maxPage = totalPages - 1;
  
  // Clamp page to valid range
  noInterrupts();
  if (encoderPosition < 0) encoderPosition = 0;
  if (encoderPosition > maxPage) encoderPosition = maxPage;
  menuNav.currentPage = encoderPosition;
  menuNav.selectedIndex = menuNav.currentPage;  // keep selection in sync
  interrupts();
  
  // Trigger full redraw if page changed
  static int lastPage = -1;
  if (lastPage != menuNav.currentPage) {
    menuNav.needsFullRedraw = true;
    lastPage = menuNav.currentPage;
  }
}

// Dosing Add Menu Handler (unified editor with inline editing)
void handleDosingAddMenu() {
  const int maxIndex = 5;  // 0-5: Pump, Days, Time, Amount, Save, Cancel

  // ---- Normal navigation mode ----
  if (!menuNav.inEditMode) {
    clampEncoderPosition(0, maxIndex);
    return;
  }

  // ---- Inline edit mode (NO CLAMPING) ----
  // Use menuNav.editValue as "lastInlineEncoderPos"
    int delta = encoderPosition - menuNav.editValue;

    if (delta != 0) {
      if (menuNav.selectedIndex == 0) {  // Pump field inline edit
        int p = (int)tempDosingSchedule.pumpNumber + delta;

        while (p < 1) p += 4;
        while (p > 4) p -= 4;

        tempDosingSchedule.pumpNumber = (uint8_t)p;
        }
      
      if (menuNav.selectedIndex == 1) {  // DAYS inline edit
        int idx = (int)menuNav.daySelectIndex + delta;

        // wrap 0..7 (7 == DONE)
        while (idx < 0) idx += 8;
        while (idx > 7) idx -= 8;

        menuNav.daySelectIndex = (uint8_t)idx;
    }

    if (menuNav.selectedIndex == 2) {  // Time field
      if (menuNav.editingHour) {
        int h = (int)tempDosingSchedule.hour + delta;
        while (h < 0)  h += 24;
        while (h > 23) h -= 24;
        tempDosingSchedule.hour = (uint8_t)h;
      } else {
        int m = (int)tempDosingSchedule.minute + (delta * 5);
        while (m < 0)  m += 60;
        while (m > 59) m -= 60;
        tempDosingSchedule.minute = (uint8_t)m;
      }
    }
    else if (menuNav.selectedIndex == 3) {  // Amount field
    int newAmt = (int)tempDosingSchedule.amountML + (delta * 1); // tenths

    if (newAmt < 0) newAmt = 5000;
    else if (newAmt > 5000) newAmt = 0;

      tempDosingSchedule.amountML = newAmt;
    }

    menuNav.editValue = encoderPosition;  // update inline reference
    menuNav.needsRedraw = true;
  }
}

void handleOutletAddMenu() {
  static int lastPos = 0;
  int pos = encoderPosition;
  int delta = pos - lastPos;
  lastPos = pos;

  //const int maxIndex = 7;
  const int maxIndex = 6; //reduced from 7 to 6

  if (!menuNav.inEditMode) {
    // normal navigation
    clampEncoderPosition(0, maxIndex);
    menuNav.selectedIndex = encoderPosition;
    menuNav.scrollOffset = 0;
    return;
  }

  // editing
  switch(menuNav.selectedIndex) {
    case 0: { // Relay number
      if (delta != 0) {
        int r = (int)tempOutletSchedule.relayNumber + delta;
        r = constrain(r, 1, 4); // adjust if you have more relays
        tempOutletSchedule.relayNumber = r;
        menuNav.needsRedraw = true;
      }
    } break;

    case 1: { // Days cursor move Su..Sa + DONE
      int idx = (int)menuNav.daySelectIndex + delta;
      while (idx < 0) idx += 8;
      while (idx > 7) idx -= 8;
      menuNav.daySelectIndex = (uint8_t)idx;
    } break;

    case 2: { // Interval editor (only when interval mode ON)
      if (!tempOutletSchedule.isInterval) return;

      // If editingHour==true => editing UNIT by a toggle on rotation
      if (menuNav.editingHour) {
        if (delta != 0) {
          menuNav.outletIntervalIsHours = !menuNav.outletIntervalIsHours;
          encoderPosition = 0;
          lastPos = 0;
          menuNav.needsRedraw = true;
        }
      } else {
        // editing numeric value
        if (delta != 0) {
          int v = (int)menuNav.outletIntervalValue + delta;
          int vMax = menuNav.outletIntervalIsHours ? 24 : 59;
          v = constrain(v, 1, vMax);
          menuNav.outletIntervalValue = v;
          menuNav.needsRedraw = true;
        }
      }
    } break;

    case 3: { // Time ON (only if not interval)
      if (tempOutletSchedule.isInterval) return;
      if (delta != 0) {
        if (menuNav.editingHour) {
          int h = (int)tempOutletSchedule.hourOn + delta;
          h = constrain(h, 0, 23);
          tempOutletSchedule.hourOn = h;
        } else {
          int m = (int)tempOutletSchedule.minuteOn + delta;
          m = constrain(m, 0, 59);
          tempOutletSchedule.minuteOn = m;
        }
        menuNav.needsRedraw = true;
      }
    } break;

    case 4: { // Time OFF (only if not interval)
      if (tempOutletSchedule.isInterval) return;
      if (delta != 0) {
        if (menuNav.editingHour) {
          int h = (int)tempOutletSchedule.hourOff + delta;
          h = constrain(h, 0, 23);
          tempOutletSchedule.hourOff = h;
        } else {
          int m = (int)tempOutletSchedule.minuteOff + delta;
          m = constrain(m, 0, 59);
          tempOutletSchedule.minuteOff = m;
        }
        menuNav.needsRedraw = true;
      }
    } break;

    default:
      break;
  }
}

// Dosing Delete Menu Handler (page-based with item selection)
void handleDosingDeleteMenu() {
  if (dosingScheduleCount == 0) {
    // No schedules to delete - just stay at position 0
    encoderPosition = 0;
    menuNav.selectedIndex = 0;
    return;
  }
  
  // Allow selection of schedules + "Return to Menu" option
  // Index 0 to dosingScheduleCount-1 = schedules
  // Index dosingScheduleCount = "Return to Menu"
  const int maxIndex = dosingScheduleCount;  // Include return option
  clampEncoderPosition(0, maxIndex);
  
  // Calculate which page the selected item is on
  const int SCHEDULES_PER_PAGE = 6;
  int newPage = menuNav.selectedIndex / SCHEDULES_PER_PAGE;
  
  // Trigger full redraw if page changed
  static int lastPage = -1;
  if (lastPage != newPage) {
    menuNav.currentPage = newPage;
    menuNav.needsFullRedraw = true;
    lastPage = newPage;
  }
}

// Day Selection Menu Handler
void handleDaySelectionMenu() {
  const int maxIndex = 7;  // 0-7: 7 days + Done button
  noInterrupts();
  if (encoderPosition < 0) encoderPosition = 0;
  if (encoderPosition > maxIndex) encoderPosition = maxIndex;
  menuNav.daySelectIndex = encoderPosition;
  interrupts();
}

// Manual Dosing Menu Handler
void handleManualDosingMenu() {
  const int maxIndex = manualDosingMenuCount - 1;  // 0-3
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Outlet Schedule Menu Handler
void handleOutletScheduleMenu() {
  const int maxIndex = outletScheduleMenuCount - 1;  // 0-3
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

void handleOutletViewMenu() {
  const int SCHEDULES_PER_PAGE = 6;
  int totalPages = (outletScheduleCount + SCHEDULES_PER_PAGE - 1) / SCHEDULES_PER_PAGE;
  if (totalPages > 4) totalPages = 4;
  if (totalPages == 0) totalPages = 1;

  const int maxPage = totalPages - 1;

  noInterrupts();
  if (encoderPosition < 0) encoderPosition = 0;
  if (encoderPosition > maxPage) encoderPosition = maxPage;

  menuNav.currentPage = encoderPosition;
  menuNav.selectedIndex = menuNav.currentPage;   // ✅ CRITICAL parity line

  interrupts();

  static int lastPage = -1;
  if (lastPage != menuNav.currentPage) {
    menuNav.needsFullRedraw = true;
    lastPage = menuNav.currentPage;
  }
}
// Outlet Delete Menu Handler (page-based with item selection)
// void handleOutletDeleteMenu() {
//   if (outletScheduleCount == 0) {
//     // No schedules to delete
//     encoderPosition        = 0;
//     menuNav.selectedIndex  = 0;
//     menuNav.currentPage    = 0;
//     return;
//   }

//   // 0..outletScheduleCount-1 = schedules
//   // outletScheduleCount       = "Return to Menu"
//   const int maxIndex = outletScheduleCount;
//   clampEncoderPosition(0, maxIndex);

//   // *** THIS is what was missing before ***
//   menuNav.selectedIndex = encoderPosition;

//   const int SCHEDULES_PER_PAGE = 6;   // must match drawOutletScheduleListScreen()
//   int newPage = menuNav.selectedIndex / SCHEDULES_PER_PAGE;

//   if (newPage != menuNav.currentPage) {
//     menuNav.currentPage     = newPage;
//     menuNav.needsFullRedraw = true;
//     menuNav.needsRedraw     = true;
//     menuNav.lastDrawnIndex  = -1;
//   } else {
//     menuNav.needsRedraw = true;
//   }
// }
// Delete Outlet – list navigation (reuses view-style list)
void handleOutletDeleteMenu() {
  if (outletScheduleCount == 0) {
    // Nothing to delete
    noInterrupts();
    encoderPosition = 0;
    interrupts();

    menuNav.selectedIndex  = 0;
    menuNav.currentPage    = 0;
    menuNav.needsRedraw    = true;
    menuNav.needsFullRedraw= true;
    return;
  }

  // 0..outletScheduleCount-1 = real schedules
  // outletScheduleCount       = "Return to Menu"
  const int maxIndex = outletScheduleCount;

  clampEncoderPosition(0, maxIndex);   // updates encoderPosition and selectedIndex

  const int SCHEDULES_PER_PAGE = 6;    // must match drawOutletScheduleListScreen()
  int newPage = menuNav.selectedIndex / SCHEDULES_PER_PAGE;

  if (newPage != menuNav.currentPage) {
    menuNav.currentPage     = newPage;
    menuNav.needsFullRedraw = true;
    menuNav.needsRedraw     = true;
    menuNav.lastDrawnIndex  = -1;
  } else {
    menuNav.needsRedraw = true;
  }
}

// Pump Calibration Menu Handler
void handlePumpCalibrationMenu() {
  const int maxIndex = pumpCalibrationMenuCount - 1;  // 0-4
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Calibrate Menu Handler (for P1-P4)
void handleCalibrateMenu() {
  const int maxIndex = calibrateConfirmMenuCount - 1;  // 0-1
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Top-Up Menu Handler
void handleTopUpMenu() {
  const int maxIndex = topupMenuCount - 1;  // 0-2
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Replace Solution Menu Handler
void handleReplaceSolutionMenu() {
  const int maxIndex = replaceMenuCount - 1;  // 0-4
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

// Confirm Menu Handler (Yes/No dialogs)
void handleConfirmMenu() {
  const int maxIndex = confirmYesNoMenuCount - 1;  // 0-1
  clampEncoderPosition(0, maxIndex);
  //menuNav.selectedIndex = hardware.encoderPosition;
}

void selectMainMenu() {
  switch (menuNav.selectedIndex) {
    case 0: navigateToMenu(MENU_SCHEDULING); break;
    case 1: navigateToMenu(MENU_MANUAL_DOSING); break;
    case 2: navigateToMenu(MENU_PUMP_CALIBRATION); break;
    case 3: navigateToMenu(MENU_TOPUP_SOLUTION); break;
    case 4: navigateToMenu(MENU_REPLACE_SOLUTION); break;
    case 5: navigateToMenu(MENU_RESET_WIFI_CONFIRM); break;
    case 6: navigateToMenu(MENU_FACTORY_RESET_CONFIRM); break;
  }
}

void selectSchedulingMenu() {
  switch (menuNav.selectedIndex) {
    case 0: navigateToMenu(MENU_DOSING_SCHEDULE); break;
    case 1: navigateToMenu(MENU_OUTLET_SCHEDULE); break;
    case 2: navigateToMenu(MENU_MAIN); break;
  }
}

void selectDosingScheduleMenu() {
  switch (menuNav.selectedIndex) {
    case 0: navigateToMenu(MENU_DOSING_VIEW); break;
    case 1:
      tempDosingSchedule.pumpNumber = 1;
      tempDosingSchedule.hour = 8;
      tempDosingSchedule.minute = 0;
      tempDosingSchedule.amountML = 10;
      menuNav.tempDaysBitmap = 0;
      navigateToMenu(MENU_DOSING_ADD, 0);
      break;
    case 2: navigateToMenu(MENU_DOSING_DELETE); break;
    case 3: navigateToMenu(MENU_DOSING_DELETE_ALL); break;
    case 4: navigateToMenu(MENU_SCHEDULING); break;
  }
}

void selectDosingViewMenu() {
  navigateToMenu(MENU_DOSING_SCHEDULE);
}

// void selectDosingErrorNoDaysMenu() {
//   // Single-item error menu: any press returns to the dosing add editor,
//   // with the Days field selected so the user can immediately fix it.
//   menuNav.inEditMode = false;
//   navigateToMenu(MENU_DOSING_ADD, 1);  // 1 = Days row in unified editor
// }

// For now these just fall through to legacy flows later,
// but we register them as selectFns to prove the pattern.
void selectManualDosingMenu()        { /* keep legacy for now */ }

void selectOutletViewMenu() {
  navigateToMenu(MENU_OUTLET_SCHEDULE);
}

void selectOutletAddSelectDaysMenu() {
  int idx = menuNav.daySelectIndex;

  if (idx >= 0 && idx < 7) {
    toggleDay(menuNav.tempDaysBitmap, idx);
    tempOutletSchedule.daysOfWeek = menuNav.tempDaysBitmap;
    menuNav.needsRedraw = true;
  } 
  else if (idx == 7) {
    // Done -> back to outlet editor, Days row
    navigateToMenu(MENU_OUTLET_ADD, 1);
  }
}

void selectOutletScheduleMenu() {
  switch (menuNav.selectedIndex) {
    case 0: // View Schedules
      navigateToMenu(MENU_OUTLET_VIEW, 0);
      break;

    case 1: // Add Schedule
      navigateToMenu(MENU_OUTLET_ADD, 0);
      break;

    case 2: // Delete Schedule
      // Go directly to the delete list menu
      navigateToMenu(MENU_OUTLET_DELETE_SELECT, 0);
      break;

    case 3: // Delete All
      navigateToMenu(MENU_OUTLET_DELETE_ALL, 0);
      break;

    case 4: // Back
      navigateToMenu(MENU_SCHEDULING);
      break;
  }
}

void selectOutletAddMenu() {
  switch(menuNav.selectedIndex) {

  case 0:  // Relay inline edit
    if (!menuNav.inEditMode) {
      // ENTER edit
      menuNav.inEditMode = true;

      // Start editing from the current relay value
      noInterrupts();
      encoderPosition = tempOutletSchedule.relayNumber - 1;  // relay is 1..4, position is 0..3
      interrupts();
    } else {
      // EXIT edit – restore cursor to Relay row
      menuNav.inEditMode = false;

      noInterrupts();
      encoderPosition = menuNav.selectedIndex;  // prevents jumping to index 0
      interrupts();
    }

    menuNav.needsRedraw = true;
    break;

  case 1: // Days inline edit (match dosing behavior)
    if (!menuNav.inEditMode) {
      // ENTER days edit
      menuNav.tempDaysBitmap = tempOutletSchedule.daysOfWeek; // seed from current
      menuNav.inEditMode = true;
      menuNav.daySelectIndex = 0; // start at Sunday

      noInterrupts();
      encoderPosition = 0;  // reset inline cursor movement
      interrupts();
    } 
    else {
      // Already editing days
      if (menuNav.daySelectIndex == 7) {
        // DONE -> EXIT edit
        menuNav.inEditMode = false;

        noInterrupts();
        encoderPosition = menuNav.selectedIndex; // restore highlight to Days row
        interrupts();
      } 
      else {
        // TOGGLE current day bit
        toggleDay(menuNav.tempDaysBitmap, menuNav.daySelectIndex);
        tempOutletSchedule.daysOfWeek = menuNav.tempDaysBitmap; // keep schedule synced
      }
    }

    menuNav.needsRedraw = true;
    break;

    case 2: // Interval toggle / edit
      if (!menuNav.inEditMode) {
        // first press toggles interval mode ON/OFF
        tempOutletSchedule.isInterval = !tempOutletSchedule.isInterval;

        if (tempOutletSchedule.isInterval) {
          // seed UI temps from stored minutes
          menuNav.outletIntervalIsHours = (tempOutletSchedule.intervalMinutes >= 60);
          menuNav.outletIntervalValue =
              menuNav.outletIntervalIsHours
                ? max(1, (int)(tempOutletSchedule.intervalMinutes / 60))
                : max(1, (int)tempOutletSchedule.intervalMinutes);

          // now enter edit mode on UNIT first
          menuNav.inEditMode = true;
          menuNav.editingHour = true; // USING THIS AS "editing unit"
          encoderPosition = 0;
        }
      } else {
        // in edit mode:
        if (menuNav.editingHour) {
          // unit -> switch to value mode
          menuNav.editingHour = false;
          encoderPosition = 0;
        } else {
          // value -> commit interval and exit edit
          tempOutletSchedule.intervalMinutes =
              menuNav.outletIntervalIsHours
                ? (uint16_t)menuNav.outletIntervalValue * 60
                : (uint16_t)menuNav.outletIntervalValue;

          menuNav.inEditMode = false;
          menuNav.editingHour = true; // reset for time fields

          // ✅ restore cursor to Interval row to prevent jump
          noInterrupts();
          encoderPosition = menuNav.selectedIndex;  // selectedIndex is 2 here
          interrupts();
        }
      }
      menuNav.needsRedraw = true;
      break;

    case 3: // Time ON
      if (tempOutletSchedule.isInterval) break;
      if (!menuNav.inEditMode) {
        menuNav.inEditMode = true;
        menuNav.editingHour = true;
        encoderPosition = 0;
      } else {
        // toggle hour/minute, then exit
        if (menuNav.editingHour) {
          menuNav.editingHour = false;
        } else {
          menuNav.inEditMode = false;
          menuNav.editingHour = true;
        }
      }
      menuNav.needsRedraw = true;
      break;

    case 4: // Time OFF
      if (tempOutletSchedule.isInterval) break;
      if (!menuNav.inEditMode) {
        menuNav.inEditMode = true;
        menuNav.editingHour = true;
        encoderPosition = 0;
      } else {
        if (menuNav.editingHour) {
          menuNav.editingHour = false;
        } else {
          menuNav.inEditMode = false;
          menuNav.editingHour = true;
        }
      }
      menuNav.needsRedraw = true;
      break;

      case 5: {
      if (menuNav.tempDaysBitmap == 0) {
        showSplash("SELECT DAYS!");
        return;
      }
      if (outletScheduleCount >= MAX_OUTLET_SCHEDULES) {
        showSplash("LIST FULL!");
        return;
      }

      tempOutletSchedule.daysOfWeek = menuNav.tempDaysBitmap;
      tempOutletSchedule.enabled = true;

      outletSchedules[outletScheduleCount++] = tempOutletSchedule;

      saveSchedulesToStorage();
    //  saveOutletSchedulesToFile();   // single truth for outlet
      showSplash("SAVED!");

      navigateToMenu(MENU_OUTLET_SCHEDULE);
    } break;

    case 6: // Cancel
      navigateToMenu(MENU_OUTLET_SCHEDULE);
      break;
  }
}

void enterOutletAddValues() {
  tempOutletSchedule.relayNumber = menuNav.tempRelay;
  tempOutletSchedule.daysOfWeek  = menuNav.tempDaysBitmap;

  // defaults
  tempOutletSchedule.hourOn = 6;
  tempOutletSchedule.minuteOn = 0;
  tempOutletSchedule.hourOff = 22;
  tempOutletSchedule.minuteOff = 0;

  tempOutletSchedule.isInterval = false;
  tempOutletSchedule.intervalMinutes = 60; // default 1 hour
  menuNav.outletIntervalIsHours = true;

  tempOutletSchedule.enabled = true;

  editingField = 0;
  navigateToMenu(MENU_OUTLET_ADD_VALUES);
}

void selectOutletAddValuesMenu() {
    switch (menuNav.selectedIndex) {
        case 0:
            // Confirmation of adding schedule
            navigateToMenu(MENU_OUTLET_ADD_CONFIRM);
            break;
        case 1:
            navigateToMenu(MENU_OUTLET_ADD);
            break;
    }
}
void selectOutletAddConfirmMenu() {
    switch (menuNav.selectedIndex) {
        case 0:
            // Save outlet schedule to storage
            outletSchedules[outletScheduleCount++] = tempOutletSchedule;
            saveSchedulesToStorage();
//            saveOutletSchedulesToFile();
            navigateToMenu(MENU_OUTLET_SCHEDULE);
            break;
        case 1:
            navigateToMenu(MENU_OUTLET_SCHEDULE);
            break;
    }
}
void selectOutletDeleteSelectMenu() {
  // No schedules -> just go back to Outlet Schedule menu
  if (outletScheduleCount == 0) {
    navigateToMenu(MENU_OUTLET_SCHEDULE);
    return;
  }

  // Last index = "Press to Return"
  if (menuNav.selectedIndex == outletScheduleCount) {
    navigateToMenu(MENU_OUTLET_SCHEDULE);
    return;
  }

  // Otherwise it's a real schedule row
  if (menuNav.selectedIndex >= 0 && menuNav.selectedIndex < outletScheduleCount) {
    menuNav.tempIndex = menuNav.selectedIndex;   // which schedule to delete
    navigateToMenu(MENU_OUTLET_DELETE_CONFIRM, 0); // start confirm at "Yes"
  }
}

// Delete Outlet – YES/NO confirm
void selectOutletDeleteConfirmMenu() {
  if (menuNav.selectedIndex == 0) {
    // Yes: delete selected entry
    int deleteIdx = menuNav.tempIndex;

    for (int i = deleteIdx; i < outletScheduleCount - 1; i++) {
      outletSchedules[i] = outletSchedules[i + 1];
    }

    outletScheduleCount--;
    saveSchedulesToStorage();    
//    saveOutletSchedulesToFile();
  }

  // In both Yes/No cases, go back to Outlet Schedule main menu
  navigateToMenu(MENU_OUTLET_SCHEDULE);
}

void selectOutletDeleteAllMenu() {
    switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_OUTLET_DELETE_ALL_CONFIRM); break;
        case 1: navigateToMenu(MENU_OUTLET_SCHEDULE); break;
    }
}
void selectOutletDeleteAllConfirmMenu() {
    switch (menuNav.selectedIndex) {
        case 0:
            outletScheduleCount = 0;
            saveSchedulesToStorage();
            navigateToMenu(MENU_OUTLET_SCHEDULE);
            break;
        case 1:
            navigateToMenu(MENU_OUTLET_SCHEDULE);
            break;
    }
}

void selectPumpCalibrationMenu()     { /* keep legacy for now */ }
void selectTopUpMenu()              { /* keep legacy for now */ }
void selectReplaceMenu()            { /* keep legacy for now */ }

void handleMenuSelection() {
  const MenuDef& m = MENUS[menuNav.currentMenu];
  if (m.selectFn) m.selectFn();
  else handleMenuSelectionLegacy();
}
static void handleMenuSelectionLegacy() {
  switch (menuNav.currentMenu) {
    case MENU_MAIN:
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_SCHEDULING); break;
        case 1: navigateToMenu(MENU_MANUAL_DOSING); break;
        case 2: navigateToMenu(MENU_PUMP_CALIBRATION); break;
        case 3: navigateToMenu(MENU_TOPUP_SOLUTION); break;
        case 4: navigateToMenu(MENU_REPLACE_SOLUTION); break;
        case 5: navigateToMenu(MENU_RESET_WIFI_CONFIRM); break;
        case 6: navigateToMenu(MENU_FACTORY_RESET_CONFIRM); break;
      }
      break;

    case MENU_SCHEDULING:
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_DOSING_SCHEDULE); break;
        case 1: navigateToMenu(MENU_OUTLET_SCHEDULE); break;
        case 2: navigateToMenu(MENU_MAIN); break;
      }
      break;

    case MENU_DOSING_SCHEDULE:
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_DOSING_VIEW); break;
        case 1:
          tempDosingSchedule.pumpNumber = 1;
          tempDosingSchedule.hour = 8;
          tempDosingSchedule.minute = 0;
          tempDosingSchedule.amountML = 10;
          menuNav.tempDaysBitmap = 0;
          navigateToMenu(MENU_DOSING_ADD, 0);
          break;
        case 2: navigateToMenu(MENU_DOSING_DELETE); break;
        case 3: navigateToMenu(MENU_DOSING_DELETE_ALL); break;
        case 4: navigateToMenu(MENU_SCHEDULING); break;
      }
      break;

    // DOSING VIEW - View saved schedules (button goes back)
    case MENU_DOSING_VIEW:
      // Button press goes back to menu
      navigateToMenu(MENU_DOSING_SCHEDULE);
      break;

    case MENU_OUTLET_VIEW:
      // Button press exits back to Outlet Schedule menu
      navigateToMenu(MENU_OUTLET_SCHEDULE);
      break;

    case MENU_DOSING_DELETE_ALL:
      if (menuNav.selectedIndex == 0) { // Yes
        // Delete all dosing schedules
        for (int i = 0; i < MAX_DOSING_SCHEDULES; i++) {
          dosingSchedules[i].enabled = false;
        }
        dosingScheduleCount = 0;
        saveSchedulesToStorage();
      }
      navigateToMenu(MENU_DOSING_SCHEDULE);
      break;

    // DOSING DELETE - Select schedule to delete
    case MENU_DOSING_DELETE:
      if (dosingScheduleCount == 0) {
        // No schedules, go back
        navigateToMenu(MENU_DOSING_SCHEDULE);
      } else if (menuNav.selectedIndex == dosingScheduleCount) {
        // Selected "Return to Menu" option
        navigateToMenu(MENU_DOSING_SCHEDULE);
      } else {
        // Store selected schedule index for deletion
        menuNav.tempPumpNumber = menuNav.selectedIndex;
        // Open confirmation dialog for selected schedule
        navigateToMenu(MENU_DOSING_DELETE_CONFIRM, 1);  // Start at "No" (safer default)
      }
      break;

    // DOSING DELETE CONFIRM - Yes/No dialog
    case MENU_DOSING_DELETE_CONFIRM:
      if (menuNav.selectedIndex == 0) { // Yes - Delete
        int deleteIdx = menuNav.tempPumpNumber;  // We'll store the selected schedule index here
        
        // Shift all schedules after this one down
        for (int i = deleteIdx; i < dosingScheduleCount - 1; i++) {
          dosingSchedules[i] = dosingSchedules[i + 1];
        }
        
        // Clear the last schedule
        dosingSchedules[dosingScheduleCount - 1].enabled = false;
        dosingScheduleCount--;
        
        // Save to storage
        saveSchedulesToStorage();
        
        // Show success message
        tft.fillScreen(BLACK);
        tft.setTextSize(1);
        tft.setCursor(20, 50);
        tft.setTextColor(GREEN);
        tft.print("DELETED!");
        delay(1000);
        
        navigateToMenu(MENU_DOSING_SCHEDULE);
      } else { // No - Cancel
        navigateToMenu(MENU_DOSING_DELETE, menuNav.tempPumpNumber);  // Return to delete list at same position
      }
      break;

    // DOSING ADD - Unified Editor
    case MENU_DOSING_ADD:
      switch (menuNav.selectedIndex) {
        case 0: // Pump (inline edit)
          if (!menuNav.inEditMode) {
            // ENTER edit
            menuNav.inEditMode = true;

            noInterrupts();
            encoderPosition = 0;
            interrupts();
            menuNav.editValue = 0;
          } else {
            // EXIT edit (confirm)
            menuNav.inEditMode = false;

            noInterrupts();
            encoderPosition = menuNav.selectedIndex; // restore highlight
            interrupts();
            menuNav.editValue = 0;
          }

          menuNav.needsRedraw = true;
          break;

        case 1: // Days (inline edit Option A)
          if (!menuNav.inEditMode) {
            // ENTER days edit
            menuNav.inEditMode = true;
            menuNav.daySelectIndex = 0; // start at Sunday (change to 1 if you want Monday)

            noInterrupts();
            encoderPosition = 0;
            interrupts();
            menuNav.editValue = 0;
          } 
          else {
            // Already editing days
            if (menuNav.daySelectIndex == 7) {
              // DONE -> EXIT edit
              menuNav.inEditMode = false;

              noInterrupts();
              encoderPosition = menuNav.selectedIndex; // restore highlight to Days row
              interrupts();
              menuNav.editValue = 0;
            } 
            else {
              // TOGGLE current day bit
              uint8_t bit = (1 << menuNav.daySelectIndex);
              menuNav.tempDaysBitmap ^= bit;
              tempDosingSchedule.daysOfWeek = menuNav.tempDaysBitmap; // keep schedule synced
            }
          }

          menuNav.needsRedraw = true;
          break;

        case 2: // Time - toggle editing hour/minute, or increment
          // First click: start editing hour
          // Second click: switch to minute
          // Third click: done editing
          if (!menuNav.inEditMode) {
            menuNav.inEditMode = true;
            menuNav.editingHour = true;
          } else if (menuNav.editingHour) {
            menuNav.editingHour = false;  // Switch to minute
          } else {
            menuNav.inEditMode = false;  // Done editing
            menuNav.editingHour = true;  // Reset for next time
          }
          menuNav.needsRedraw = true;
          break;
        case 3: // Amount - toggle editing or increment
          menuNav.inEditMode = !menuNav.inEditMode;
          menuNav.needsRedraw = true;
          break;
        // case 4: // Save
        //   {
        //       // Validate
        //       if (menuNav.tempDaysBitmap == 0) {
        //           // Show error
        //           tft.fillScreen(BLACK);
        //           tft.setTextColor(RED);
        //           tft.setTextSize(1);
        //           tft.setCursor(10, 50);
        //           tft.print("ERROR:SELECT DAYS!");
        //           delay(2000);
        //           menuNav.needsFullRedraw = true;
        //           break;
        //       }

        //       if (dosingScheduleCount >= MAX_DOSING_SCHEDULES) {
        //           // Array full error
        //           tft.fillScreen(BLACK);
        //           tft.setTextColor(RED);
        //           tft.setTextSize(1);
        //           tft.setCursor(10, 50);
        //           tft.print("ERROR: LIST FULL!");
        //           delay(2000);
        //           menuNav.needsFullRedraw = true;
        //           break;
        //       }

        //       // Save to array
        //       tempDosingSchedule.daysOfWeek = menuNav.tempDaysBitmap;
        //       tempDosingSchedule.enabled = true;
        //       tempDosingSchedule.isInterval = false;
        //       tempDosingSchedule.intervalMinutes = 0;

        //       dosingSchedules[dosingScheduleCount] = tempDosingSchedule;
        //       dosingScheduleCount++;

        //       // Save to EEPROM (NOT JSON)
        //       saveSchedulesToStorage();

        //       // Show success
        //       tft.fillScreen(BLACK);
        //       tft.setTextSize(2);
        //       tft.setCursor(20, 50);
        //       tft.setTextColor(GREEN);
        //       tft.print("SAVED!");
        //       delay(1500);

        //       menuNav.inEditMode = false;
        //       navigateToMenu(MENU_DOSING_SCHEDULE);
        //   }
        //   break;  
        case 4: // Save
        {
            // Validate
            // if (menuNav.tempDaysBitmap == 0) {
            //     // ✅ Go to a dedicated error screen that can be dismissed with a press
            //     menuNav.inEditMode = false;
            //     navigateToMenu(MENU_DOSING_ERROR_NO_DAYS);
            //     break;
            // }

            if (menuNav.tempDaysBitmap == 0) {
                showSplash("SELECT DAYS!");
                break;
            }

            if (dosingScheduleCount >= MAX_DOSING_SCHEDULES) {
                // Array full error (still uses a blocking splash)
                tft.fillScreen(BLACK);
                tft.setTextColor(RED);
                tft.setTextSize(1);
                tft.setCursor(10, 50);
                tft.print("ERROR: LIST FULL!");
                delay(2000);
                menuNav.needsFullRedraw = true;
                break;
            }

            // Normal save path...
            tempDosingSchedule.daysOfWeek   = menuNav.tempDaysBitmap;
            tempDosingSchedule.enabled      = true;
            tempDosingSchedule.isInterval   = false;
            tempDosingSchedule.intervalMinutes = 0;

            dosingSchedules[dosingScheduleCount] = tempDosingSchedule;
            dosingScheduleCount++;

            saveSchedulesToStorage();

            tft.fillScreen(BLACK);
            tft.setTextSize(2);
            tft.setCursor(20, 50);
            tft.setTextColor(GREEN);
            tft.print("SAVED!");
            delay(1500);

            menuNav.inEditMode = false;
            navigateToMenu(MENU_DOSING_SCHEDULE);
        }
        break;

        case 5: // Cancel
          menuNav.inEditMode = false;  // Exit edit mode
          navigateToMenu(MENU_DOSING_SCHEDULE);
          break;
      }
      break;

    // DOSING ADD - Day Selection
    case MENU_DOSING_ADD_SELECT_DAYS:
      {
        int idx = menuNav.daySelectIndex;

        if (idx >= 0 && idx < 7) {
          // Toggle day 0-6
          toggleDay(menuNav.tempDaysBitmap, idx);
          menuNav.needsRedraw = true;
          // Don't set needsFullRedraw - partial update is enough
        }
        else if (idx == 7) {
          // Done - return to unified editor on Days line
          navigateToMenu(MENU_DOSING_ADD, 1);  // Line 1 = Days
        }
      }
      break;

    // DOSING ADD - Time Selection
    case MENU_DOSING_ADD_SET_TIME:
      // Toggle between editing hour and minute
      if (menuNav.editingHour) {
        menuNav.editingHour = false;  // Switch to minute
        menuNav.needsRedraw = true;
        // Partial redraw - no flicker
      } else {
        // Done with time, return to unified editor on Time line
        menuNav.editingHour = true;  // Reset for next time
        navigateToMenu(MENU_DOSING_ADD, 2);  // Line 2 = Time
      }
      break;

    // DOSING ADD - Amount Selection
    case MENU_DOSING_ADD_SET_AMOUNT:
      // Done - return to unified editor on Amount line
      navigateToMenu(MENU_DOSING_ADD, 3);  // Line 3 = Amount
      break;

    // MANUAL DOSING MENU
    case MENU_MANUAL_DOSING:
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_MANUAL_SELECT_PUMP); break;
        case 1: navigateToMenu(MENU_MANUAL_SET_AMOUNT); break;
          menuNav.needsFullRedraw = true;
          break;
        case 2: // Start Dosing
          break;
        case 3: // Cancel
          menuNav.currentMenu = MENU_MAIN;
          menuNav.selectedIndex = 0;
          menuNav.scrollOffset = 0;
          menuNav.needsFullRedraw = true;
          break;
      }
      break;

    // OUTLET SCHEDULE MENU
    case MENU_OUTLET_SCHEDULE:
      switch (menuNav.selectedIndex) {
      //  case 0: navigateToMenu(MENU_OUTLET_VIEW);         break;
        case 0: navigateToMenu(MENU_OUTLET_ADD);          break;
        case 1: navigateToMenu(MENU_OUTLET_DELETE_SELECT);break;  // <-- FIXED
        case 2: navigateToMenu(MENU_OUTLET_DELETE_ALL);   break;
        case 3: navigateToMenu(MENU_SCHEDULING);          break;
      }
    break;

    case MENU_OUTLET_DELETE_ALL:
      if (menuNav.selectedIndex == 0) { // Yes
        // Clear all outlet schedules
        for (int i = 0; i < MAX_OUTLET_SCHEDULES; i++) {
          outletSchedules[i].enabled = false;
        }
        // ✅ IMPORTANT: count must go to zero
        outletScheduleCount = 0;
        // Keep Preferences in sync (since boot still loads prefs)
        saveSchedulesToStorage();
      }
      navigateToMenu(MENU_OUTLET_SCHEDULE);
      break;

    // PUMP CALIBRATION MENU
    case MENU_PUMP_CALIBRATION:
      menuNav.tempPumpNumber = menuNav.selectedIndex + 1; // Store pump number
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_CALIBRATE_P1); break;
        case 1: navigateToMenu(MENU_CALIBRATE_P2); break;
        case 2: navigateToMenu(MENU_CALIBRATE_P3); break;
          menuNav.tempPumpNumber = 3;
          navigateToMenu(MENU_CALIBRATE_P3);
          break;
        case 3:
          menuNav.tempPumpNumber = 4;
          navigateToMenu(MENU_CALIBRATE_P4);
          break;
        case 4: navigateToMenu(MENU_MAIN); break; // Back
      }
      break;

    // Handle all calibration menus (P1-P4) similarly
    case MENU_CALIBRATE_P1:
    case MENU_CALIBRATE_P2:
    case MENU_CALIBRATE_P3:
    case MENU_CALIBRATE_P4:
      if (menuNav.selectedIndex == 0) { // Start Calibration
        menuNav.isCalibrating = true;
        // TODO: Start calibration wizard
      } else { // Cancel
        navigateToMenu(MENU_PUMP_CALIBRATION);
      }
      break;

    // TOP-UP SOLUTION MENU
    case MENU_TOPUP_SOLUTION:
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_TOPUP_SET_AMOUNTS); break;
        case 1: navigateToMenu(MENU_TOPUP_SET_PUMP_PIN); break;
        case 2: navigateToMenu(MENU_MAIN); break; // Back
      }
      break;

    // REPLACE SOLUTION MENU
    case MENU_REPLACE_SOLUTION:
      switch (menuNav.selectedIndex) {
        case 0: navigateToMenu(MENU_REPLACE_SET_AMOUNTS); break;
        case 1: navigateToMenu(MENU_REPLACE_SET_DRAIN); break;
        case 2: navigateToMenu(MENU_REPLACE_SET_FILL); break;
        case 3: navigateToMenu(MENU_REPLACE_SET_SCHEDULE); break;
        case 4: navigateToMenu(MENU_MAIN); break; // Back
      }
      break;

    // RESET WIFI CONFIRM
    case MENU_RESET_WIFI_CONFIRM:
    if (menuNav.selectedIndex == 0) { // Yes
        // Clear WiFi credentials from EEPROM
        Preferences wifiPrefs;
        wifiPrefs.begin("wifi", false);
        wifiPrefs.clear();
        wifiPrefs.end();
        delay(1000);
        ESP.restart();
    } else { // No
        navigateToMenu(MENU_MAIN);
        menuNav.needsFullRedraw = true;
      }
      break;

    // FACTORY RESET CONFIRM
    case MENU_FACTORY_RESET_CONFIRM:
      if (menuNav.selectedIndex == 0) { // Yes
        preferences.begin("schedules", false);
        preferences.clear();
        preferences.end();
        preferences.begin("pumps", false);
        preferences.clear();
        preferences.end();
        preferences.begin("topup", false);
        preferences.clear();
        preferences.end();
        preferences.begin("replace", false);
        preferences.clear();
        preferences.end();
        preferences.begin("config", false);
        preferences.clear();
        preferences.end();
        //wm.resetSettings();
        delay(1000);
        ESP.restart();
      } else { // No
        navigateToMenu(MENU_MAIN);
      }
      break;
    default:
      // no-op
      break;
  }
}

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);
  // DISABLE WATCHDOG GLOBALLY FOR TESTING
 // Send multiple test messages
  Serial.println();
  Serial.println();
  Serial.println("==================================");
  Serial.println("SERIAL TEST - IF YOU SEE THIS, SERIAL WORKS!");
  Serial.println("==================================");
  Serial.println();

  Serial.println("\n=== Hydroponics Controller v3.0 ===");
  Serial.println("Initializing...\n");

  // DEBUG: Relay 1 - Boot started
  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, HIGH);
  delay(200);
  digitalWrite(RELAY_1, LOW);

  // Initialize LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Initialize LittleFS
  if (!initLittleFS()) {
    Serial.println("      ERROR: LittleFS initialization failed!");
  } else {
  }

  // Initialize TFT
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);
  initDisplay();

  // DEBUG: Relay 2 - Display initialized
  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_2, HIGH);
  delay(200);
  digitalWrite(RELAY_2, LOW);

  // Initialize Hardware (Relays, Pumps, Sensors)
  initHardware();

  // Initialize Rotary Encoder (MUST be in main.cpp with ISR)
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  // Read initial encoder state
  int MSB = digitalRead(ENCODER_DT);
  int LSB = digitalRead(ENCODER_CLK);
  lastEncoded = (MSB << 1) | LSB;

  // Attach encoder interrupts (MUST be where ISR is defined)
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);

  // Initialize RTC
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!rtc.begin()) {
    Serial.println("      ERROR: DS3231 not found!");
    updateMqttStatus("RTC Error");
  } else {
    if (rtc.lostPower()) {
    }
  }

  // Load configuration from LittleFS
  if (!loadConfigFromLittleFS()) {
    setDefaultConfig();
    saveConfigToLittleFS();
  }
   // Initialize WiFi using SimpleWiFi module
  initWiFi();
  
  // Feed watchdog after WiFi initialization
  esp_task_wdt_reset();

  // After WiFi initialization, setup MQTT if connected
  if (WiFi.status() == WL_CONNECTED) {
    mqtt.setServer(config.mqttBroker.c_str(), config.mqttPort);
    mqtt.setCallback(mqttCallback);
    Serial.println("MQTT configured - connection will happen in loop");
  }
  
  // Check connection status
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    currentData.ip = WiFi.localIP().toString();
    updateWifiStatus("Connected");
    Serial.print("✓ WiFi connected. IP: ");
    Serial.println(currentData.ip);
    
    // DEBUG: Turn on LED 1 when WiFi connects
    setLED(1, true);
    
    // Feed watchdog before NTP sync
    esp_task_wdt_reset();

    // Start NTP sync immediately when WiFi connects (don't wait for MQTT)
    Serial.println("Starting NTP sync...");
    startNTPSync();
    
    // Feed watchdog after NTP sync initiated
    esp_task_wdt_reset();

    // Initialize MQTT separately (with timeout protection)
    mqtt.setServer(config.mqttBroker.c_str(), config.mqttPort);
    mqtt.setCallback(mqttCallback);

    // DEBUG: LED 4 indicates MQTT is configured (connection will happen async in MQTTTask)
    setLED(4, true);
    
    Serial.println("MQTT configured - connection will happen async in MQTTTask");
    // DON'T call connectMQTT() here - let MQTTTask handle it to avoid watchdog timeout
    // connectMQTT();
  } else {
    Serial.println("Running in standalone mode (no WiFi)");
    updateWifiStatus("Standalone");
  }

  // Initialize Web Server
  setupWebServer();
  server.begin();
  
  // DEBUG: LED 3 blinks to indicate web server started
  setLED(3, true);
  delay(200);
  setLED(3, false);
  delay(200);
  setLED(3, true);
  delay(200);
  setLED(3, false);

  // Initialize menu system
  initMenuSystem();
  menuNav.lastActivity = millis();
  menuNav.needsFullRedraw = true;
  menuNav.needsRedraw = true;

  // Draw initial menu immediately
  drawMenu();
  
  // DEBUG: Relay 1 turns ON when menu is drawn
  setRelay(1, true);
  delay(500);
  setRelay(1, false);
  
  // Feed watchdog before creating tasks
  esp_task_wdt_reset();

  // DEBUG: Relay 2 turns ON when all tasks are created
  setRelay(2, true);
  delay(500);
  setRelay(2, false);
  
  Serial.println("\n=== Setup Complete ===");
  Serial.println("Entering main loop...\n");
}

// ============================================
// MAIN LOOP
// ============================================
// ============================================
// DOSING SCHEDULE EXECUTION ENGINE
// ============================================

// Dosing execution state machine
enum DosingState {
  DOSING_IDLE,
  DOSING_RUNNING,
  DOSING_COMPLETE
};

struct DosingExecution {
  DosingState state = DOSING_IDLE;
  uint8_t activePump = 0;
  unsigned long startTime = 0;
  unsigned long runDuration = 0;
  uint16_t targetML = 0;
  uint8_t scheduleIndex = 0;
};

DosingExecution activeDosing;

// Track last execution time for each schedule (prevent duplicate runs)
unsigned long lastDosingExecution[MAX_DOSING_SCHEDULES] = {0};
const unsigned long DOSING_COOLDOWN = 60000; // 1 minute cooldown

// Check schedules (call every 1 second from loop)
void checkDosingSchedules(unsigned long currentTime) {
  // Don't check while dosing is running
  if (activeDosing.state == DOSING_RUNNING) return;
  
  // Check every second
  static unsigned long lastCheck = 0;
  if (currentTime - lastCheck < 1000) return;
  lastCheck = currentTime;
  
  // Get current time from RTC
  DateTime now = rtc.now();
  
  // Check each enabled schedule
  for (int i = 0; i < MAX_DOSING_SCHEDULES; i++) {
    DosingSchedule &sched = dosingSchedules[i];
    
    // Skip if disabled
    if (!sched.enabled) continue;
    
    // Skip if not today
    if (!isScheduleActiveToday(sched.daysOfWeek)) continue;
    
    // Skip if not current time (hour:minute match)
    if (sched.hour != now.hour() || sched.minute != now.minute()) continue;
    
    // Skip if recently executed (within cooldown)
    if ((currentTime - lastDosingExecution[i]) < DOSING_COOLDOWN) continue;
    
    // MATCH! Start dosing
    startDosing(i, currentTime);
    break; // Only execute one schedule at a time
  }
}

// Start dosing operation
void startDosing(uint8_t scheduleIndex, unsigned long currentTime) {
  DosingSchedule &sched = dosingSchedules[scheduleIndex];
  PumpCalibration &cal = pumpCalibrations[sched.pumpNumber - 1];
  
  // Calculate runtime based on calibration
  float mlPerSec = cal.mlPerSecond;
  float targetML = sched.amountML / 10.0; // Convert from tenths
  unsigned long runMs = (unsigned long)((targetML / mlPerSec) * 1000);
  
  // Start pump
  activeDosing.state = DOSING_RUNNING;
  activeDosing.activePump = sched.pumpNumber;
  activeDosing.startTime = currentTime;
  activeDosing.runDuration = runMs;
  activeDosing.scheduleIndex = scheduleIndex;
  activeDosing.targetML = sched.amountML;
  
  setPumpSpeed(sched.pumpNumber, cal.pwmSpeed);
  
  // Mark as executed
  lastDosingExecution[scheduleIndex] = currentTime;
  
  // Optional: Log to serial for debugging
  Serial.print("[DOSING] Starting Pump ");
  Serial.print(sched.pumpNumber);
  Serial.print(" for ");
  Serial.print(targetML, 1);
  Serial.print(" mL (");
  Serial.print(runMs);
  Serial.println(" ms)");
}

// Update dosing state machine (call every loop iteration)
void updateDosingExecution(unsigned long currentTime) {
  if (activeDosing.state != DOSING_RUNNING) return;
  
  // Check if duration complete
  if ((currentTime - activeDosing.startTime) >= activeDosing.runDuration) {
    // Stop pump
    setPumpSpeed(activeDosing.activePump, 0);
    
    // Log completion
    Serial.print("[DOSING] Completed Pump ");
    Serial.print(activeDosing.activePump);
    Serial.print(" - ");
    Serial.print(activeDosing.targetML / 10.0, 1);
    Serial.println(" mL dispensed");
    
    // Reset state
    activeDosing.state = DOSING_COMPLETE;
    activeDosing.activePump = 0;
    
    // Transition to IDLE after short delay
    delay(100); // Brief pause before next schedule can run
    activeDosing.state = DOSING_IDLE;
  }
}

// ============================================
// MAIN LOOP (Runs on Core 0)
// ============================================
// Core 0: WiFi, MQTT, Encoder ISR, Networking
// Core 1: Display rendering, Float switches, Touch sensors
void loop() {
  esp_task_wdt_reset();
  
  if (pendingRestart && millis() > restartAt) {
    ESP.restart();
  }

  unsigned long currentTime = millis();

  checkMenuTimeout(currentTime);

  // Handle WiFi state and reconnection
  handleWiFi();
  // Check and execute dosing schedules
  checkDosingSchedules(currentTime);
  updateDosingExecution(currentTime);
  
  // Heartbeat LED
  if ((unsigned long)(currentTime - lastHeartbeat) >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = currentTime;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }

  // Update encoder
  updateEncoder();
  // Handle menu navigation and redraw
  handleMenuNavigation();
  drawMenu();

  // DEBUG: Removed LED 2/3 pin state display - was interfering with menu
  // LED debugging confirmed encoder IS working

  // Update status bar
  updateStatusBar();

  // Handle WiFi state changes
  handleWiFiState(currentTime);

  // Handle MQTT with robust state machine
  handleMQTT();

  // Publish MQTT sensor data if connected
  if (mqtt.connected()) {
    publishSensorData(currentTime);
  }

  // Check for daily NTP sync at midnight (and once on boot if connected)
  checkDailySync(currentTime);

  // Write sensor log to LittleFS
  if (config.enableLogging && spiffsReady) {
    if ((unsigned long)(currentTime - lastLogWrite) >= LOG_WRITE_INTERVAL) {
      lastLogWrite = currentTime;
      logSensorData();
    }
  }

  // Update sensor data for web interface
  if ((unsigned long)(currentTime - lastWebUpdate) >= WEB_UPDATE_INTERVAL) {
    lastWebUpdate = currentTime;
    updateSensorData();
    notifyWebClients();
  }

  ws.cleanupClients();
  yield();
}

// ============================================
// WIFI STATE HANDLING
// ============================================
void handleWiFiState(unsigned long currentTime) {
  if (isAPMode()) return;
  static unsigned long lastCheck = 0;
  if ((unsigned long)(currentTime - lastCheck) < 5000) return;
  lastCheck = currentTime;

  bool isConnected = (WiFi.status() == WL_CONNECTED);

  if (isConnected && !wifiConnected) {
    // WiFi just connected
    wifiConnected = true;
    currentData.ip = WiFi.localIP().toString();
    updateWifiStatus("Connected");
    lastWifiReconnect = currentTime;  // Reset reconnect timer

    // Start NTP sync when WiFi reconnects
    startNTPSync();

    // Reconnect MQTT
    mqtt.setServer(config.mqttBroker.c_str(), config.mqttPort);
    mqtt.setCallback(mqttCallback);
    connectMQTT();

  } else if (!isConnected && wifiConnected) {
    // WiFi just disconnected
    wifiConnected = false;
    mqttConnected = false;
    updateWifiStatus("Disconnected");
    updateMqttStatus("Offline");
    lastWifiReconnect = currentTime;  // Start reconnect timer

    Serial.println("WiFi disconnected - will retry in 30 minutes");

  } else if (!isConnected && !wifiConnected) {
    // Still disconnected - check if it's time to retry
    if ((unsigned long)(currentTime - lastWifiReconnect) >= WIFI_RECONNECT_INTERVAL) {
      Serial.println("Attempting WiFi reconnection...");
      WiFi.reconnect();
      lastWifiReconnect = currentTime;  // Reset timer for next attempt
    }
  }
}

// ============================================
// ONLINE SERVICES
// ============================================
void initOnlineServices() {

  // Start NTP sync (non-blocking)
  startNTPSync();

  mqtt.setServer(config.mqttBroker.c_str(), config.mqttPort);
  mqtt.setCallback(mqttCallback);
  connectMQTT();

}

// ============================================
// NTP SYNC STATE MACHINE
// ============================================
enum NTPSyncState {
  NTP_IDLE,
  NTP_SYNCING,
  NTP_CHECKING,
  NTP_SUCCESS,
  NTP_FAILED
};

NTPSyncState ntpSyncState = NTP_IDLE;
unsigned long ntpSyncStartTime = 0;
int ntpSyncAttempts = 0;
const int NTP_MAX_ATTEMPTS = 10;
const unsigned long NTP_CHECK_INTERVAL = 1000;

void startNTPSync() {
  if (ntpSyncState != NTP_IDLE) return;

  updateNtpStatus("Syncing");

  configTime(UTC_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  ntpSyncState = NTP_SYNCING;
  ntpSyncStartTime = millis();
  ntpSyncAttempts = 0;
}

void updateNTPSync() {
  if (ntpSyncState == NTP_IDLE || ntpSyncState == NTP_SUCCESS || ntpSyncState == NTP_FAILED) {
    return;
  }

  unsigned long currentTime = millis();

  // Check if we should attempt to read time
  if (ntpSyncState == NTP_SYNCING) {
    if ((unsigned long)(currentTime - ntpSyncStartTime) >= NTP_CHECK_INTERVAL) {
      ntpSyncState = NTP_CHECKING;
    }
  }

  // Attempt to read time
  if (ntpSyncState == NTP_CHECKING) {
    // Feed watchdog during NTP check
    esp_task_wdt_reset();
    
    time_t now = time(nullptr);

    if (now > 100000) {
      // Success!
      struct tm timeinfo;
      localtime_r(&now, &timeinfo);

      DateTime newTime(timeinfo.tm_year + 1900,
                       timeinfo.tm_mon + 1,
                       timeinfo.tm_mday,
                       timeinfo.tm_hour,
                       timeinfo.tm_min,
                       timeinfo.tm_sec);

      rtc.adjust(newTime);
      lastNTPSyncTime = newTime;
      lastNTPSync = millis();
      ntpSynced = true;

      // Update last sync display time
      lastSyncHour = newTime.hour();
      lastSyncMinute = newTime.minute();

      updateNtpStatus("Synced");

      ntpSyncState = NTP_SUCCESS;
      
      // DEBUG: Turn on LED 2 when NTP syncs
      setLED(2, true);
    } else {
      // Not ready yet, try again
      ntpSyncAttempts++;

      if (ntpSyncAttempts >= NTP_MAX_ATTEMPTS) {
        // Failed after max attempts
        Serial.println("NTP sync failed");
        updateNtpStatus("Failed");
        ntpSyncState = NTP_FAILED;
      } else {
        // Try again
        ntpSyncState = NTP_SYNCING;
        ntpSyncStartTime = currentTime;
      }
    }
  }
}

void resetNTPSync() {
  ntpSyncState = NTP_IDLE;
  ntpSyncAttempts = 0;
}

// ============================================
// NTP FUNCTIONS
// ============================================

void checkDailySync(unsigned long currentTime) {
  if (!wifiConnected) return;
  
  static bool hasRunOnBoot = false;
  
  // Run once on first boot if WiFi is connected
  if (!hasRunOnBoot) {
    hasRunOnBoot = true;
    Serial.println("Running initial NTP sync on boot...");
    updateNTPSync();  // Run the sync state machine once
    return;
  }
  
  // Then check every minute for midnight trigger
  if ((unsigned long)(currentTime - lastDailySyncCheck) < DAILY_SYNC_CHECK_INTERVAL) return;

  lastDailySyncCheck = currentTime;
  DateTime now = rtc.now();

  // Trigger at midnight (00:00)
  if (now.hour() == 0 && now.minute() == 0) {
    if ((millis() - lastNTPSync) > 3600000) {  // Only if last sync was >1 hour ago
      Serial.println("Midnight NTP sync triggered");
      resetNTPSync();
      startNTPSync();
      updateNTPSync();  // Process the sync
    }
  }
}

// ============================================
// HARDWARE CONTROL FUNCTIONS
// ============================================

// Robust MQTT handler with state machine
void handleMQTT() {
  unsigned long currentTime = millis();
  
  // Safety: Disable MQTT after too many failures
  if (mqttFailCount >= MAX_MQTT_FAILURES) {
    if (mqttState != MQTT_STATE_DISABLED) {
      mqttState = MQTT_STATE_DISABLED;
      setLED(3, false);
    }
    return;
  }

  // Rate limit checks
  if (currentTime - lastMqttCheck < MQTT_CHECK_INTERVAL) return;
  lastMqttCheck = currentTime;

  switch (mqttState) {
    case MQTT_STATE_DISCONNECTED:
      if (!wifiConnected || WiFi.status() != WL_CONNECTED) return;
      
      mqttState = MQTT_STATE_CONNECTING;
      mqttConnectStart = currentTime;
      connectMQTT();
      break;
      
    case MQTT_STATE_CONNECTING:
      if (currentTime - mqttConnectStart > MQTT_CONNECT_TIMEOUT) {
        mqtt.disconnect();
        mqttFailCount++;
        mqttState = MQTT_STATE_FAILED;  // Go to FAILED state instead
        setLED(3, false);
        return;
      }
      
      if (mqtt.connected()) {
        mqttState = MQTT_STATE_CONNECTED;
        mqttFailCount = 0;
        setLED(3, true); //set as true by default
      }
      break;
      
    case MQTT_STATE_CONNECTED:
      if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
        mqtt.disconnect();
        mqttState = MQTT_STATE_DISCONNECTED;
        setLED(3, false); //false by default
        return;
      }
      
      if (!mqtt.connected()) {
        mqttState = MQTT_STATE_DISCONNECTED;
        setLED(3, false);
        return;
      }
      
      mqtt.loop();
      break;
      
    case MQTT_STATE_FAILED:
      // Wait 30 seconds before retry (non-blocking)
      if (currentTime - mqttConnectStart > 30000) {
        mqttState = MQTT_STATE_DISCONNECTED;
      }
      break;
      
    case MQTT_STATE_DISABLED:
      break;
  }
}

// ============================================
// MQTT FUNCTIONS
// ============================================
void connectMQTT() {
  if (!wifiConnected) return;

  // Simple TLS setup (like test code)
  espClientSecure.setInsecure();
  mqtt.setClient(espClientSecure);
  mqtt.setServer(config.mqttBroker.c_str(), config.mqttPort);
  mqtt.setCallback(mqttCallback);
  
  String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  // Simple connection attempt (like test code)
  bool connected;
  if (config.mqttUser.length() > 0) {
    connected = mqtt.connect(clientId.c_str(), config.mqttUser.c_str(), config.mqttPass.c_str());
  } else {
    connected = mqtt.connect(clientId.c_str());
  }
  
  if (connected) {
    mqttConnected = true;
    
    // Subscribe to test LED topic
    String testLedTopic = config.mqttTopic + "/test/led";
    mqtt.subscribe(testLedTopic.c_str());
    
    // Subscribe to additional topics if configured
    if (config.mqttSubTopic1.length() > 0) {
      mqtt.subscribe(config.mqttSubTopic1.c_str());
    }
    if (config.mqttSubTopic2.length() > 0) {
      mqtt.subscribe(config.mqttSubTopic2.c_str());
    }
    if (config.mqttSubTopic3.length() > 0) {
      mqtt.subscribe(config.mqttSubTopic3.c_str());
    }
  } else {
    mqttConnected = false;
  }
}

void reconnectMQTT(unsigned long currentTime) {
  if ((unsigned long)(currentTime - lastMqttReconnect) < MQTT_RECONNECT_INTERVAL) return;

  // Only attempt reconnect if WiFi is connected
  if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
    return;
  }

  lastMqttReconnect = currentTime;

  // Don't block - just try once quickly
  Serial.println("Quick MQTT reconnect attempt...");

  // Use shorter timeout for background reconnects
  if (config.mqttUseTLS) {
    espClientSecure.setTimeout(2);  // Only 2 seconds for background attempts
  } else {
    espClient.setTimeout(2);
  }

  connectMQTT();

  // Restore normal timeout
  if (config.mqttUseTLS) {
    espClientSecure.setTimeout(5);
  } else {
    espClient.setTimeout(5);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Handle test LED topic
  String topicStr = String(topic);
  String testLedTopic = config.mqttTopic + "/test/led";

  if (topicStr == testLedTopic) {
    message.trim();
    message.toUpperCase();

    if (message == "LED ON" || message == "ON" || message == "1") {
      testLedState = true;
    } else if (message == "LED OFF" || message == "OFF" || message == "0") {
      testLedState = false;
    } else {
    }
  } else {
  }
}

void publishSensorData(unsigned long currentTime) {
  if (!mqttConnected) return;
  if ((unsigned long)(currentTime - lastMqttPublish) < config.publishInterval) return;

  lastMqttPublish = currentTime;

  DateTime now = rtc.now();
  float temp = rtc.getTemperature();

  char timeStr[30];
  sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());
  mqtt.publish((config.mqttTopic + "/time").c_str(), timeStr);

  char tempStr[10];
  dtostrf(temp, 4, 1, tempStr);
  mqtt.publish((config.mqttTopic + "/temp").c_str(), tempStr);

  mqtt.publish((config.mqttTopic + "/status").c_str(), "online");
}

// ============================================
// DISPLAY FUNCTIONS
// ============================================

void updateDisplay() {
  // DISABLED - Menu system now handles all display rendering
  // This function is no longer used to prevent overlap with menu
  return;
}

void updateWifiStatus(const char* status) {
  // DISABLED - Menu system status bar shows WiFi connection status
  // Store status for internal state only
  strcpy(lastWifiStatus, status);
}

void updateMqttStatus(const char* status) {
  // DISABLED - Menu system status bar shows MQTT connection status
  // Store status for internal state only
  strcpy(lastMqttStatus, status);
}

void updateNtpStatus(const char* status) {
  // DISABLED - Menu system status bar shows last sync time
  // Store status for internal state only
  if (strcmp(status, "Synced") == 0) {
    sprintf(lastNtpStatus, "%02d/%02d %02d:%02d",
            lastNTPSyncTime.month(),
            lastNTPSyncTime.day(),
            lastNTPSyncTime.hour(),
            lastNTPSyncTime.minute());
  } else {
    strcpy(lastNtpStatus, status);
  }
}