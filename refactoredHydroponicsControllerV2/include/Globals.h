/*
 * Globals.h
 * 
 * Global type definitions, enums, structs, constants, and extern declarations
 * for the Hydroponics Controller project.
 * 
 * This header is included by all modules to access shared types and variables.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <RTClib.h>
#include <TFT_ILI9163C.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

// ==================================================
// UTILITY MACROS
// ==================================================
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#define PMSTR(s) (__FlashStringHelper*)(s)  // Helper for PROGMEM strings

// ==================================================
// MENU SYSTEM ENUMERATIONS
// ==================================================
enum MenuState {
  MENU_MAIN,
  
  // Scheduling
  MENU_SCHEDULING,
  
  // Dosing Schedule
  MENU_DOSING_SCHEDULE,
  MENU_DOSING_VIEW,
  MENU_DOSING_ADD,
  MENU_DOSING_ADD_SELECT_DAYS,
  MENU_DOSING_ADD_SET_TIME,
  MENU_DOSING_ADD_SET_AMOUNT,
  MENU_DOSING_DELETE,
  MENU_DOSING_DELETE_CONFIRM,
  MENU_DOSING_DELETE_ALL,
//  MENU_DOSING_ERROR_NO_DAYS,   // NEW: error screen when no days selected

  // Outlet Schedule
  MENU_OUTLET_SCHEDULE,
  MENU_OUTLET_VIEW,
  MENU_OUTLET_ADD,
  MENU_OUTLET_ADD_SELECT_DAYS,   // NEW
  MENU_OUTLET_ADD_VALUES,
  MENU_OUTLET_ADD_CONFIRM,
  MENU_OUTLET_DELETE,
  MENU_OUTLET_DELETE_SELECT,
  MENU_OUTLET_DELETE_CONFIRM,
  MENU_OUTLET_DELETE_ALL,
  MENU_OUTLET_DELETE_ALL_CONFIRM,
  
  // Manual Dosing
  MENU_MANUAL_DOSING,
  MENU_MANUAL_SELECT_PUMP,
  MENU_MANUAL_SET_AMOUNT,
  MENU_MANUAL_CONFIRM,
  
  // Pump Calibration
  MENU_PUMP_CALIBRATION,
  MENU_CALIBRATE_P1,
  MENU_CALIBRATE_P1_START,
  MENU_CALIBRATE_P1_CONFIRM,
  MENU_CALIBRATE_P2,
  MENU_CALIBRATE_P2_START,
  MENU_CALIBRATE_P2_CONFIRM,
  MENU_CALIBRATE_P3,
  MENU_CALIBRATE_P3_START,
  MENU_CALIBRATE_P3_CONFIRM,
  MENU_CALIBRATE_P4,
  MENU_CALIBRATE_P4_START,
  MENU_CALIBRATE_P4_CONFIRM,
  
  // Top-up Solution
  MENU_TOPUP_SOLUTION,
  MENU_TOPUP_SET_AMOUNTS,
  MENU_TOPUP_AMOUNTS_CONFIRM,
  MENU_TOPUP_SET_PUMP_PIN,
  MENU_TOPUP_PUMP_CONFIRM,
  
  // Replace Solution
  MENU_REPLACE_SOLUTION,
  MENU_REPLACE_SET_AMOUNTS,
  MENU_REPLACE_SET_DRAIN,
  MENU_REPLACE_SET_FILL,
  MENU_REPLACE_SET_SCHEDULE,
  MENU_REPLACE_CONFIRM,
  
  // WiFi & Reset
  MENU_RESET_WIFI,
  MENU_RESET_WIFI_CONFIRM,
  MENU_FACTORY_RESET,
  MENU_FACTORY_RESET_CONFIRM
};

enum MQTTState {
  MQTT_STATE_DISCONNECTED,
  MQTT_STATE_CONNECTING,
  MQTT_STATE_CONNECTED,
  MQTT_STATE_FAILED,
  MQTT_STATE_DISABLED
};

// ==================================================
// DATA STRUCTURES
// ==================================================
struct MenuNavigationState {
  MenuState currentMenu = MENU_MAIN;
  int selectedIndex = 0;
  int lastDrawnIndex = -1;
  int scrollOffset = 0;
  bool inEditMode = false;
  int editValue = 0;
  unsigned long lastActivity = 0;
  bool needsRedraw = true;
  bool needsFullRedraw = true;
  
  // Temporary workflow values
  uint8_t tempPumpNumber = 1;
  uint16_t tempAmount = 0;
  uint8_t tempRelay = 1;
  bool isCalibrating = false;
  int tempIndex = -1;   // generic "selected schedule index" for delete flows
  
  // Dosing schedule wizard
  uint8_t tempDaysBitmap = 0;
  uint8_t daySelectIndex = 0;
  bool editingHour = true;
  
  // Pagination
  int currentPage = 0;

  // Outlet interval editor helpers (UI-only)
  bool outletIntervalIsHours = false;  // true=hours, false=minutes
  uint8_t outletIntervalValue = 1;     // 1-59 mins or 1-24 hrs


};

struct DosingSchedule {
  uint8_t pumpNumber;
  uint8_t daysOfWeek;
  uint8_t hour;
  uint8_t minute;
  uint16_t amountML;
  bool isInterval;
  uint16_t intervalMinutes;
  bool enabled;
};

struct OutletSchedule {
  uint8_t relayNumber;
  uint8_t daysOfWeek;

  // TIME mode fields
  uint8_t hourOn;
  uint8_t minuteOn;
  uint8_t hourOff;
  uint8_t minuteOff;

  // INTERVAL mode fields (NEW)
  bool isInterval;            // true = interval mode, false = fixed ON/OFF times
  uint16_t intervalMinutes;   // total minutes between ON events (1..59 or 60..1440)

  bool enabled;
};


struct PumpCalibration {
  uint8_t pwmSpeed;
  uint16_t timeMs;
  float mlPerSecond;
  bool isCalibrated;
};

struct TopUpConfig {
  uint16_t pump1ML;
  uint16_t pump2ML;
  uint16_t pump3ML;
  uint16_t pump4ML;
  uint8_t fillPumpRelay;
  bool enabled;
};

struct ReplaceConfig {
  uint16_t pump1ML;
  uint16_t pump2ML;
  uint16_t pump3ML;
  uint16_t pump4ML;
  uint8_t drainRelay;
  uint8_t fillRelay;
  uint8_t scheduleDay;
  uint8_t scheduleHour;
  bool enabled;
};

struct Config {
  String apPassword;
  String mqttBroker;
  int mqttPort;
  String mqttUser;
  String mqttPass;
  String mqttTopic;
  String mqttSubTopic1;
  String mqttSubTopic2;
  String mqttSubTopic3;
  int publishInterval;
  bool enableLogging;
  bool mqttUseTLS;
  String webUsername;
  String webPassword;
  
  // WiFi credentials (added for SimpleWiFi module)
  char wifi_ssid[33];      // Max SSID length is 32 + null terminator
  char wifi_password[65];  // Max WPA2 password is 64 + null terminator
  bool wifi_ap_mode;       // True = AP mode, False = Station mode
};

struct HardwareState {
  // Float switches
  bool floatFull = false;
  bool floatLow = false;
  bool floatEmpty = false;
  
  // Relays
  bool relay1 = false;
  bool relay2 = false;
  bool relay3 = false;
  bool relay4 = false;
  
  // Pumps (0-100%)
  uint8_t pump1Speed = 0;
  uint8_t pump2Speed = 0;
  uint8_t pump3Speed = 0;
  uint8_t pump4Speed = 0;
  
  // LEDs
  bool led1 = false;
  bool led2 = false;
  bool led3 = false;
  bool led4 = false;
  
  // WS2812B RGB
  uint8_t ws2812b_r = 0;
  uint8_t ws2812b_g = 0;
  uint8_t ws2812b_b = 0;
  
  // Encoder
  int encoderPosition = 0;
  bool encoderButton = false;
  
  // Touch sensors
  bool touch1 = false;
  bool touch2 = false;
  bool touch3 = false;
  bool touch4 = false;
};

struct SensorData {
  String timestamp;
  float temperature;
  bool wifiStatus;
  bool mqttStatus;
  int spiffsUsed;
  int spiffsTotal;
  String ip;
};

// ==================================================
// PIN DEFINITIONS
// ==================================================
// I2C
#define SDA_PIN  37
#define SCL_PIN  47

// TFT Display (SPI)
#define TFT_CS   40
#define TFT_DC   38
#define TFT_RST  39
#define TFT_MOSI 35
#define TFT_SCK  36

// Rotary Encoder
#define ENCODER_CLK  4
#define ENCODER_DT   15
#define ENCODER_SW   5

// Touch Pins
#define TOUCH_1  8
#define TOUCH_2  9
#define TOUCH_3  7
#define TOUCH_4  6

// Float Switches
#define FLOAT_FULL   21
#define FLOAT_LOW    20
#define FLOAT_EMPTY  19

// Relays
#define RELAY_1  14
#define RELAY_2  13
#define RELAY_3  12
#define RELAY_4  11

// Pumps
#define PUMP_1   16
#define PUMP_2   17
#define PUMP_3   18
#define PUMP_4   10

// LEDs
#define LED_1  41
#define LED_2  42
#define LED_3  2
#define LED_4  1

// WS2812B
#define WS2812B_PIN  48
#define WS2812B_COUNT 1

// UART
#define UART_TX  43
#define UART_RX  44

// ==================================================
// CONSTANTS
// ==================================================
// PWM
#define PWM_FREQ     1000
#define PWM_RESOLUTION 8

// Network
#define AP_NAME "Hydroponics_Controller"
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_SEC 28800
#define DAYLIGHT_OFFSET_SEC 0

// Files
#define CONFIG_FILE "/config.json"
#define LOG_FILE "/sensor_log.txt"
#define WEB_USERNAME "admin"
#define WEB_PASSWORD "hydro2024"

// Colors
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Display
#define MENU_ITEMS_PER_PAGE 8
#define MENU_TIMEOUT 30000
#define MENU_ITEM_HEIGHT 12
#define MENU_HEADER_HEIGHT 15
#define MENU_STATUS_HEIGHT 20
#define ICON_SIZE 5

// Storage
#define MAX_DOSING_SCHEDULES 24
#define MAX_OUTLET_SCHEDULES 10

// Encoder
#define PULSES_PER_DETENT 2
#define ENCODER_DEBOUNCE_MS 5
#define PULSES_PER_STEP 2

// Timing intervals (milliseconds)
#define HEARTBEAT_INTERVAL 1000
#define MQTT_RECONNECT_INTERVAL 30000
#define MQTT_PUBLISH_INTERVAL 5000
#define DISPLAY_UPDATE_INTERVAL 50
#define DAILY_SYNC_CHECK_INTERVAL 60000
#define LOG_WRITE_INTERVAL 300000
#define WEB_UPDATE_INTERVAL 1000
#define FLOAT_CHECK_INTERVAL 500
#define TOUCH_CHECK_INTERVAL 100
#define ENCODER_CHECK_INTERVAL 10
#define WIFI_RECONNECT_INTERVAL 1800000

// Misc
#define TOUCH_THRESHOLD 40
#define FLOAT_DEBOUNCE_DELAY 50
#define BUTTON_DEBOUNCE 200
#define BUTTON_LONG_PRESS_MS 1000
#define STATUS_BAR_UPDATE_INTERVAL 1000
#define MQTT_CONNECT_TIMEOUT 10000
#define MAX_MQTT_FAILURES 10
#define MQTT_CHECK_INTERVAL 1000

// FreeRTOS cores
#define CORE_0 0
#define CORE_1 1

// Day bitmaps
#define DAY_SUNDAY    0b00000001
#define DAY_MONDAY    0b00000010
#define DAY_TUESDAY   0b00000100
#define DAY_WEDNESDAY 0b00001000
#define DAY_THURSDAY  0b00010000
#define DAY_FRIDAY    0b00100000
#define DAY_SATURDAY  0b01000000
#define DAY_ALL       0b01111111
#define DAY_WEEKDAYS  0b00111110
#define DAY_WEEKENDS  0b01000001

// Forward declarations for async web server
class AsyncWebServer;
class AsyncWebSocket;
class WiFiManager;
class WiFiManagerParameter;
// ==================================================
// GLOBAL VARIABLE DECLARATIONS (extern)
// ==================================================
// System control
void scheduleRestart(unsigned long delayMs = 800);

// (optional but recommended if WebServer needs to read these)
extern volatile bool pendingRestart;
extern unsigned long restartAt;

// Day names
extern const char* dayNames[];
extern const char* dayNamesShort[];

// Menu strings (PROGMEM)
extern const char* const mainMenuItems[];
extern const int mainMenuCount;
extern const char* const schedulingMenuItems[];
extern const int schedulingMenuCount;
extern const char* const dosingScheduleMenu[];
extern const int dosingScheduleMenuCount;
extern const char* const dosingConfirmMenu[];
extern const int dosingConfirmMenuCount;
// extern const char* const dosingErrorNoDaysMenu[];
// extern const int dosingErrorNoDaysMenuCount;
extern const char* const confirmYesNoMenu[];
extern const int confirmYesNoMenuCount;
extern const char* const manualDosingMenu[];
extern const int manualDosingMenuCount;
extern const char* const outletScheduleMenu[];
extern const int outletScheduleMenuCount;
extern const char* const pumpCalibrationMenu[];
extern const int pumpCalibrationMenuCount;
extern const char* const calibrateConfirmMenu[];
extern const int calibrateConfirmMenuCount;
extern const char* const calibrateSaveMenu[];
extern const int calibrateSaveMenuCount;
extern const char* const topupMenu[];
extern const int topupMenuCount;
extern const char* const replaceMenu[];
extern const int replaceMenuCount;
extern const char* const daySelectMenuItems[];
extern const int daySelectMenuCount;

// Hardware objects
// Note: WiFiManager objects moved to main.cpp to avoid library conflicts
extern RTC_DS3231 rtc;
extern TFT_ILI9163C tft;
extern WiFiClientSecure espClientSecure;
extern WiFiClient espClient;
extern PubSubClient mqtt;
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern Adafruit_NeoPixel ws2812b;
extern Preferences preferences;

// FreeRTOS task handles
extern TaskHandle_t DisplayTaskHandle;
extern TaskHandle_t SensorTaskHandle;
extern TaskHandle_t MQTTTaskHandle;

// State variables
extern Config config;
extern HardwareState hardware;
extern MenuNavigationState menuNav;
extern SensorData currentData;
extern MQTTState mqttState;

// Storage arrays
extern DosingSchedule dosingSchedules[MAX_DOSING_SCHEDULES];
extern OutletSchedule outletSchedules[MAX_OUTLET_SCHEDULES];
extern PumpCalibration pumpCalibrations[4];
extern TopUpConfig topUpConfig;
extern ReplaceConfig replaceConfig;

// Counters
extern int dosingScheduleCount;
extern int outletScheduleCount;

// Temporary editing variables
extern DosingSchedule tempDosingSchedule;
extern OutletSchedule tempOutletSchedule;
extern uint8_t editingField;

// Encoder variables (volatile)
extern volatile long encoderPosition;
extern volatile int lastEncoded;
extern volatile unsigned long lastEncoderTime;
extern volatile int pulseCounter;
extern volatile bool encoderButtonPressed;
extern volatile unsigned long lastButtonTime;

// Button polling
extern int lastButtonState;
extern int currentButtonState;
extern unsigned long buttonPressTime;

// Float switch state
extern unsigned long lastFloatDebounce[3];
extern int lastFloatReading[3];
extern int floatState[3];

// Timing variables
extern unsigned long lastHeartbeat;
extern unsigned long lastMqttReconnect;
extern unsigned long lastMqttPublish;
extern unsigned long lastDisplayUpdate;
extern unsigned long lastDailySyncCheck;
extern unsigned long lastNTPSync;
extern unsigned long lastLogWrite;
extern unsigned long lastWebUpdate;
extern unsigned long lastWifiReconnect;
extern unsigned long lastStatusBarUpdate;
extern unsigned long lastFloatCheck;
extern unsigned long lastTouchCheck;
extern unsigned long lastEncoderCheck;
extern unsigned long lastMqttCheck;
extern unsigned long mqttConnectStart;

// Status flags
extern bool wifiConnected;
extern bool mqttConnected;
extern bool testLedState;
extern bool ntpSynced;
extern bool displayInitialized;
extern bool spiffsReady;
extern bool ledState;

// Display tracking
extern uint8_t lastDisplayedSecond;
extern uint8_t lastDisplayedMinute;
extern uint8_t lastDisplayedHour;
extern uint8_t lastDisplayedDay;
extern bool lastWifiState;
extern bool lastMqttState;
extern char lastWifiStatus[30];
extern char lastMqttStatus[30];
extern char lastNtpStatus[50];
extern DateTime lastNTPSyncTime;
extern uint8_t lastSyncHour;
extern uint8_t lastSyncMinute;

// MQTT state
extern int mqttFailCount;

// ==================================================
// DAY HELPER FUNCTIONS
// ==================================================
bool isDayEnabled(uint8_t daysBitmap, uint8_t dayIndex);
void toggleDay(uint8_t &daysBitmap, uint8_t dayIndex);
void setDay(uint8_t &daysBitmap, uint8_t dayIndex, bool enabled);
String getDaysString(uint8_t daysBitmap);
String getDaysStringLong(uint8_t daysBitmap);
String formatDaysCompact(uint8_t daysBitmap);
bool isScheduleActiveToday(uint8_t daysBitmap);

// Note: isScheduleActiveToday() is implemented in main.cpp since it requires RTC access

#endif // GLOBALS_H
