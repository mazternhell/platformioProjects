/*
 * Globals.cpp
 * 
 * Global variable definitions and PROGMEM menu string storage.
 */

#include "Globals.h"

// ==================================================
// DAY NAMES
// ==================================================
const char* dayNames[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

const char* dayNamesShort[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

// ==================================================
// PROGMEM MENU STRINGS
// ==================================================

// Main menu
const char mainMenuItems_0[] PROGMEM = "Scheduling";
const char mainMenuItems_1[] PROGMEM = "Manual Dosing";
const char mainMenuItems_2[] PROGMEM = "Pump Calibration";
const char mainMenuItems_3[] PROGMEM = "Top-up Solution";
const char mainMenuItems_4[] PROGMEM = "Replace Solution";
const char mainMenuItems_5[] PROGMEM = "Reset WiFi";
const char mainMenuItems_6[] PROGMEM = "Factory Reset";
const char* const mainMenuItems[] PROGMEM = {
  mainMenuItems_0, mainMenuItems_1, mainMenuItems_2, mainMenuItems_3,
  mainMenuItems_4, mainMenuItems_5, mainMenuItems_6
};
const int mainMenuCount = 7;

// Scheduling submenu
const char schedulingMenu_0[] PROGMEM = "Dosing Schedule";
const char schedulingMenu_1[] PROGMEM = "Outlet Schedule";
const char schedulingMenu_2[] PROGMEM = "Back";
const char* const schedulingMenuItems[] PROGMEM = {
  schedulingMenu_0, schedulingMenu_1, schedulingMenu_2
};
const int schedulingMenuCount = 3;

// Dosing schedule submenu
const char dosingScheduleMenu_0[] PROGMEM = "View Schedules";
const char dosingScheduleMenu_1[] PROGMEM = "Add Schedule";
const char dosingScheduleMenu_2[] PROGMEM = "Delete Schedule";
const char dosingScheduleMenu_3[] PROGMEM = "Delete All";
const char dosingScheduleMenu_4[] PROGMEM = "Back";
const char* const dosingScheduleMenu[] PROGMEM = {
  dosingScheduleMenu_0, dosingScheduleMenu_1, dosingScheduleMenu_2,
  dosingScheduleMenu_3, dosingScheduleMenu_4
};
const int dosingScheduleMenuCount = 5;

// Dosing confirm menu
const char dosingConfirmMenu_0[] PROGMEM = "Save to EEPROM";
const char dosingConfirmMenu_1[] PROGMEM = "Cancel";
const char* const dosingConfirmMenu[] PROGMEM = {
  dosingConfirmMenu_0, dosingConfirmMenu_1
};
const int dosingConfirmMenuCount = 2;

// Yes/No confirm menu
const char confirmYesNoMenu_0[] PROGMEM = "Yes";
const char confirmYesNoMenu_1[] PROGMEM = "No";
const char* const confirmYesNoMenu[] PROGMEM = {
  confirmYesNoMenu_0, confirmYesNoMenu_1
};
const int confirmYesNoMenuCount = 2;

// // Dosing add error: no days selected
// const char dosingErrorNoDaysMenu_0[] PROGMEM = "OK";
// const char* const dosingErrorNoDaysMenu[] PROGMEM = {
//   dosingErrorNoDaysMenu_0
// };
// const int dosingErrorNoDaysMenuCount = 1;

// // Dosing "no days selected" error menu
// const char dosingErrorNoDays_0[] PROGMEM = "ERROR: SELECT DAYS";
// const char dosingErrorNoDays_1[] PROGMEM = "Press to return";

// const char* const dosingErrorNoDaysMenu[] PROGMEM = {
//   dosingErrorNoDays_0,
//   dosingErrorNoDays_1
// };

// const int dosingErrorNoDaysMenuCount = 2;

// Manual dosing menu
const char manualDosingMenu_0[] PROGMEM = "Select Pump";
const char manualDosingMenu_1[] PROGMEM = "Set Amount (mL)";
const char manualDosingMenu_2[] PROGMEM = "Start Dosing";
const char manualDosingMenu_3[] PROGMEM = "Cancel";
const char* const manualDosingMenu[] PROGMEM = {
  manualDosingMenu_0, manualDosingMenu_1, manualDosingMenu_2, manualDosingMenu_3
};
const int manualDosingMenuCount = 4;

// Outlet Schedule Menu
const char outletScheduleItem0[] PROGMEM = "View Schedules";
const char outletScheduleItem1[] PROGMEM = "Add Schedule";
const char outletScheduleItem2[] PROGMEM = "Delete Schedule";
const char outletScheduleItem3[] PROGMEM = "Delete All";
const char outletScheduleItem4[] PROGMEM = "Back";

const char* const outletScheduleMenu[] PROGMEM = {
  outletScheduleItem0,
  outletScheduleItem1,
  outletScheduleItem2,
  outletScheduleItem3,
  outletScheduleItem4
};

const int outletScheduleMenuCount = 5;


// Pump calibration submenu
const char pumpCalibrationMenu_0[] PROGMEM = "Calibrate Pump 1";
const char pumpCalibrationMenu_1[] PROGMEM = "Calibrate Pump 2";
const char pumpCalibrationMenu_2[] PROGMEM = "Calibrate Pump 3";
const char pumpCalibrationMenu_3[] PROGMEM = "Calibrate Pump 4";
const char pumpCalibrationMenu_4[] PROGMEM = "Back";
const char* const pumpCalibrationMenu[] PROGMEM = {
  pumpCalibrationMenu_0, pumpCalibrationMenu_1, pumpCalibrationMenu_2,
  pumpCalibrationMenu_3, pumpCalibrationMenu_4
};
const int pumpCalibrationMenuCount = 5;

// Calibration confirm menu
const char calibrateConfirmMenu_0[] PROGMEM = "Start Calibration";
const char calibrateConfirmMenu_1[] PROGMEM = "Cancel";
const char* const calibrateConfirmMenu[] PROGMEM = {
  calibrateConfirmMenu_0, calibrateConfirmMenu_1
};
const int calibrateConfirmMenuCount = 2;

// Calibration save menu
const char calibrateSaveMenu_0[] PROGMEM = "Save to EEPROM";
const char calibrateSaveMenu_1[] PROGMEM = "Cancel";
const char* const calibrateSaveMenu[] PROGMEM = {
  calibrateSaveMenu_0, calibrateSaveMenu_1
};
const int calibrateSaveMenuCount = 2;

// Top-up menu
const char topupMenu_0[] PROGMEM = "Set Pump Amounts";
const char topupMenu_1[] PROGMEM = "Set Fill Relay";
const char topupMenu_2[] PROGMEM = "Back";
const char* const topupMenu[] PROGMEM = {
  topupMenu_0, topupMenu_1, topupMenu_2
};
const int topupMenuCount = 3;

// Replace solution menu
const char replaceMenu_0[] PROGMEM = "Set Pump Amounts";
const char replaceMenu_1[] PROGMEM = "Set Drain Relay";
const char replaceMenu_2[] PROGMEM = "Set Fill Relay";
const char replaceMenu_3[] PROGMEM = "Set Schedule";
const char replaceMenu_4[] PROGMEM = "Back";
const char* const replaceMenu[] PROGMEM = {
  replaceMenu_0, replaceMenu_1, replaceMenu_2, replaceMenu_3, replaceMenu_4
};
const int replaceMenuCount = 5;

// Day selection menu
const char daySelectMenu_0[] PROGMEM = "Sunday";
const char daySelectMenu_1[] PROGMEM = "Monday";
const char daySelectMenu_2[] PROGMEM = "Tuesday";
const char daySelectMenu_3[] PROGMEM = "Wednesday";
const char daySelectMenu_4[] PROGMEM = "Thursday";
const char daySelectMenu_5[] PROGMEM = "Friday";
const char daySelectMenu_6[] PROGMEM = "Saturday";
const char daySelectMenu_7[] PROGMEM = "All Days";
const char daySelectMenu_8[] PROGMEM = "Weekdays";
const char daySelectMenu_9[] PROGMEM = "Weekends";
const char daySelectMenu_10[] PROGMEM = "Done";
const char* const daySelectMenuItems[] PROGMEM = {
  daySelectMenu_0, daySelectMenu_1, daySelectMenu_2, daySelectMenu_3,
  daySelectMenu_4, daySelectMenu_5, daySelectMenu_6, daySelectMenu_7,
  daySelectMenu_8, daySelectMenu_9, daySelectMenu_10
};
const int daySelectMenuCount = 11;

// ==================================================
// HARDWARE OBJECTS
// ==================================================
// Note: WiFiManager objects moved to main.cpp to avoid library conflicts
RTC_DS3231 rtc;
TFT_ILI9163C tft = TFT_ILI9163C(TFT_CS, TFT_DC, TFT_RST);
WiFiClientSecure espClientSecure;
WiFiClient espClient;
PubSubClient mqtt(espClient);
//AsyncWebServer server(80);
//AsyncWebSocket ws("/ws");
Adafruit_NeoPixel ws2812b(WS2812B_COUNT, WS2812B_PIN, NEO_GRB + NEO_KHZ800);
Preferences preferences;

// ==================================================
// FREERTOS TASK HANDLES
// ==================================================
TaskHandle_t DisplayTaskHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
TaskHandle_t MQTTTaskHandle = NULL;

// ==================================================
// STATE VARIABLES
// ==================================================
Config config;
HardwareState hardware;
MenuNavigationState menuNav;
SensorData currentData;
MQTTState mqttState = MQTT_STATE_DISCONNECTED;

// ==================================================
// STORAGE ARRAYS
// ==================================================
DosingSchedule dosingSchedules[MAX_DOSING_SCHEDULES];
OutletSchedule outletSchedules[MAX_OUTLET_SCHEDULES];
PumpCalibration pumpCalibrations[4];
TopUpConfig topUpConfig;
ReplaceConfig replaceConfig;

// ==================================================
// COUNTERS
// ==================================================
int dosingScheduleCount = 0;
int outletScheduleCount = 0;

// ==================================================
// TEMPORARY EDITING VARIABLES
// ==================================================
DosingSchedule tempDosingSchedule;
OutletSchedule tempOutletSchedule;
uint8_t editingField = 0;

// ==================================================
// ENCODER VARIABLES (volatile)
// ==================================================
volatile long encoderPosition = 0;
volatile int lastEncoded = 0;
volatile unsigned long lastEncoderTime = 0;
volatile int pulseCounter = 0;
volatile bool encoderButtonPressed = false;
volatile unsigned long lastButtonTime = 0;

// ==================================================
// BUTTON POLLING
// ==================================================
int lastButtonState = HIGH;
int currentButtonState = HIGH;
unsigned long buttonPressTime = 0;

// ==================================================
// FLOAT SWITCH STATE
// ==================================================
unsigned long lastFloatDebounce[3] = {0, 0, 0};
int lastFloatReading[3] = {LOW, LOW, LOW};
int floatState[3] = {LOW, LOW, LOW};

// ==================================================
// TIMING VARIABLES
// ==================================================
unsigned long lastHeartbeat = 0;
unsigned long lastMqttReconnect = 0;
unsigned long lastMqttPublish = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDailySyncCheck = 0;
unsigned long lastNTPSync = 0;
unsigned long lastLogWrite = 0;
unsigned long lastWebUpdate = 0;
unsigned long lastWifiReconnect = 0;
unsigned long lastStatusBarUpdate = 0;
unsigned long lastFloatCheck = 0;
unsigned long lastTouchCheck = 0;
unsigned long lastEncoderCheck = 0;
unsigned long lastMqttCheck = 0;
unsigned long mqttConnectStart = 0;

// ==================================================
// STATUS FLAGS
// ==================================================
bool wifiConnected = false;
bool mqttConnected = false;
bool testLedState = false;
bool ntpSynced = false;
bool displayInitialized = false;
bool spiffsReady = false;
bool ledState = false;

// ==================================================
// DISPLAY TRACKING
// ==================================================
uint8_t lastDisplayedSecond = 255;
uint8_t lastDisplayedMinute = 255;
uint8_t lastDisplayedHour = 255;
uint8_t lastDisplayedDay = 255;
bool lastWifiState = false;
bool lastMqttState = false;
char lastWifiStatus[30] = "";
char lastMqttStatus[30] = "";
char lastNtpStatus[50] = "";
DateTime lastNTPSyncTime;
uint8_t lastSyncHour = 0;
uint8_t lastSyncMinute = 0;

// ==================================================
// MQTT STATE
// ==================================================
int mqttFailCount = 0;

// ==================================================
// OTHERS
// ==================================================

// ==================================================
// DAY HELPER FUNCTION IMPLEMENTATIONS
// ==================================================
bool isDayEnabled(uint8_t daysBitmap, uint8_t dayIndex) {
  return (daysBitmap & (1 << dayIndex)) != 0;
}

void toggleDay(uint8_t &daysBitmap, uint8_t dayIndex) {
  daysBitmap ^= (1 << dayIndex);
}

void setDay(uint8_t &daysBitmap, uint8_t dayIndex, bool enabled) {
  if (enabled) {
    daysBitmap |= (1 << dayIndex);
  } else {
    daysBitmap &= ~(1 << dayIndex);
  }
}

String getDaysString(uint8_t daysBitmap) {
  if (daysBitmap == DAY_ALL) return "All Days";
  if (daysBitmap == DAY_WEEKDAYS) return "Weekdays";
  if (daysBitmap == DAY_WEEKENDS) return "Weekends";
  if (daysBitmap == 0) return "None";

  String result = "";
  for (int i = 0; i < 7; i++) {
    if (isDayEnabled(daysBitmap, i)) {
      if (result.length() > 0) result += ",";
      result += dayNamesShort[i];
    }
  }
  return result;
}

String getDaysStringLong(uint8_t daysBitmap) {
  if (daysBitmap == DAY_ALL) return "All Days";
  if (daysBitmap == DAY_WEEKDAYS) return "Mon-Fri";
  if (daysBitmap == DAY_WEEKENDS) return "Sat-Sun";
  if (daysBitmap == 0) return "No Days";

  String result = "";
  for (int i = 0; i < 7; i++) {
    if (isDayEnabled(daysBitmap, i)) {
      if (result.length() > 0) result += ", ";
      result += dayNames[i];
    }
  }
  return result;
}

String formatDaysCompact(uint8_t daysBitmap) {
  if (daysBitmap == DAY_ALL) return "All Days";
  if (daysBitmap == DAY_WEEKDAYS) return "Mo-Fr";
  if (daysBitmap == DAY_WEEKENDS) return "Sa,Su";
  if (daysBitmap == 0) return "None";

  String result = "";
  const char* shortNames[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

  for (int i = 0; i < 7; i++) {
    if (isDayEnabled(daysBitmap, i)) {
      if (result.length() > 0) result += ",";
      result += shortNames[i];
    }
  }
  return result;
}

// Note: isScheduleActiveToday() implementation requires RTC access,
// so it stays in main.cpp where rtc is accessible
