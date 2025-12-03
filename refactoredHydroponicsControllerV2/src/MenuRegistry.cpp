// MenuRegistry.cpp
#include "MenuRegistry.h"
#include "DisplayUI.h"

// ---- Extern menu item arrays/counts from Globals.cpp ----
extern const char* const mainMenuItems[];
extern const int mainMenuCount;

extern const char* const schedulingMenuItems[];
extern const int schedulingMenuCount;

extern const char* const dosingScheduleMenu[];
extern const int dosingScheduleMenuCount;

extern const char* const manualDosingMenu[];
extern const int manualDosingMenuCount;

extern const char* const outletScheduleMenu[];
extern const int outletScheduleMenuCount;

extern const char* const pumpCalibrationMenu[];
extern const int pumpCalibrationMenuCount;

extern const char* const topupMenu[];
extern const int topupMenuCount;

extern const char* const replaceMenu[];
extern const int replaceMenuCount;

// extern const char* const dosingErrorNoDaysMenu[];
// extern const int dosingErrorNoDaysMenuCount;

// ---- Select/Nav functions implemented in main.cpp ----
extern void selectMainMenu();
extern void selectSchedulingMenu();
extern void selectDosingScheduleMenu();
extern void selectManualDosingMenu();
extern void selectOutletScheduleMenu();
extern void selectPumpCalibrationMenu();
extern void selectTopUpMenu();
extern void selectReplaceMenu();
extern void selectOutletAddMenu();
extern void selectDosingViewMenu();
extern void selectOutletAddSelectDaysMenu();
extern void selectOutletDeleteAllMenu();
extern void selectOutletDeleteAllConfirmMenu();
extern void selectOutletDeleteSelectMenu();
extern void selectOutletDeleteConfirmMenu();
//extern void selectDosingErrorNoDaysMenu();


// Custom nav handlers already exist in main.cpp
extern void handleDosingViewMenu();
extern void handleDosingAddMenu();
extern void handleDaySelectionMenu();
extern void handleConfirmMenu();
extern void handleCalibrateMenu();
extern void handleOutletViewMenu();
extern void handleOutletAddMenu();
extern void handleOutletDeleteMenu();

//draw nav handlers
extern void drawOutletScheduleListScreen();

// Helper macro for placeholder menus (legacy draw/select/nav)
#define MENU_PLACEHOLDER(_title, _back) \
  { _title, nullptr, 0, false, nullptr, nullptr, nullptr, _back }

const MenuDef MENUS[] = {
  // MENU_MAIN
  {
    "MAIN MENU",
    mainMenuItems,
    mainMenuCount,
    true,
    drawMainMenu,        // keep your custom main renderer
    nullptr,             // default nav
    selectMainMenu,      // new tiny select fn
    MENU_MAIN
  },

  // MENU_SCHEDULING
  {
    "SCHEDULING",
    schedulingMenuItems,
    schedulingMenuCount,
    true,
    nullptr,             // generic draw
    nullptr,
    selectSchedulingMenu,
    MENU_MAIN
  },

  // MENU_DOSING_SCHEDULE
  {
    "DOSING SCHEDULE",
    dosingScheduleMenu,
    dosingScheduleMenuCount,
    true,
    nullptr,
    nullptr,
    selectDosingScheduleMenu,
    MENU_SCHEDULING
  },

// MENU_DOSING_VIEW
  {
    "DOSING VIEW",
    nullptr, 0, false,
    drawDosingScheduleListScreen,
    handleDosingViewMenu,
    selectDosingViewMenu,     // <-- instead of nullptr
    MENU_DOSING_SCHEDULE
  },

  // MENU_DOSING_ADD (unified editor)
  {
    "DOSING ADD",
    nullptr, 0, false,
    drawScheduleEditorScreen,
    handleDosingAddMenu,
    nullptr,
    MENU_DOSING_SCHEDULE
  },

  // MENU_DOSING_ADD_SELECT_DAYS (custom days UI)
  {
    "SELECT DAYS",
    nullptr, 0, false,
    drawDaySelectionScreen,
    handleDaySelectionMenu,
    nullptr,
    MENU_DOSING_ADD
  },

  // MENU_DOSING_ADD_SET_TIME (custom time UI)
  {
    "SET TIME",
    nullptr, 0, false,
    drawTimeSelectionScreen,
    nullptr,
    nullptr,
    MENU_DOSING_ADD
  },

  // MENU_DOSING_ADD_SET_AMOUNT (custom amount UI)
  {
    "SET AMOUNT",
    nullptr, 0, false,
    drawAmountSelectionScreen,
    nullptr,
    nullptr,
    MENU_DOSING_ADD
  },

  // MENU_DOSING_DELETE (custom delete list)
  {
    "DOSING DELETE",
    nullptr, 0, false,
    drawDosingDeleteListScreen,
    nullptr,
    nullptr,
    MENU_DOSING_SCHEDULE
  },

  // MENU_DOSING_DELETE_CONFIRM
  MENU_PLACEHOLDER("DELETE CONFIRM", MENU_DOSING_DELETE),

  // MENU_DOSING_DELETE_ALL
  MENU_PLACEHOLDER("DELETE ALL DOSING", MENU_DOSING_SCHEDULE),

  // MENU_OUTLET_SCHEDULE
  {
    "OUTLET SCHEDULE",
    outletScheduleMenu,
    outletScheduleMenuCount,
    true,
    nullptr,
    nullptr,
    selectOutletScheduleMenu,
    MENU_SCHEDULING
  },

  // MENU_OUTLET_VIEW
  {
    "OUTLET VIEW",
    nullptr, 0, false,
    drawOutletScheduleListScreen,
    handleOutletViewMenu,
    nullptr,                 // press exits via legacy or custom select if you want
    MENU_OUTLET_SCHEDULE
  },

  // MENU_OUTLET_ADD
  {
    "OUTLET ADD",
    nullptr, 0, false,
    drawOutletEditorScreen,
    handleOutletAddMenu,
    selectOutletAddMenu,
    MENU_OUTLET_SCHEDULE
  },

  // MENU_OUTLET_ADD_SELECT_DAYS  ✅ MISSING BEFORE
  {
    "SELECT DAYS",
    nullptr, 0, false,
    drawDaySelectionScreen,
    handleDaySelectionMenu,
    selectOutletAddSelectDaysMenu,   // we add this in main.cpp
    MENU_OUTLET_ADD
  },

  // MENU_OUTLET_ADD_SET_TIME (if you still use it; else placeholder)
  MENU_PLACEHOLDER("OUTLET SET TIME", MENU_OUTLET_ADD),

  // MENU_OUTLET_ADD_VALUES
  MENU_PLACEHOLDER("OUTLET VALUES", MENU_OUTLET_ADD),

  // MENU_OUTLET_ADD_CONFIRM
  MENU_PLACEHOLDER("OUTLET ADD CONFIRM", MENU_OUTLET_ADD),

  // MENU_OUTLET_DELETE (custom delete list, reuse outlet view renderer)
  {
    "OUTLET DELETE",
    nullptr, 0, false,
    drawOutletScheduleListScreen,
    nullptr,
    nullptr,
    MENU_OUTLET_SCHEDULE
  },

// MENU_OUTLET_DELETE_SELECT
  {
    "OUTLET DELETE SELECT",
    nullptr, 0, false,
    drawOutletScheduleListScreen,   // reuse the same list screen as VIEW
    handleOutletDeleteMenu,         // encoder → selectedIndex + paging
    selectOutletDeleteSelectMenu,   // press → confirm/back
    MENU_OUTLET_SCHEDULE
  },

  // MENU_OUTLET_DELETE_ALL  (Yes/No prompt)
  {
    "DELETE ALL OUTLET",
    confirmYesNoMenu,
    confirmYesNoMenuCount,
    false,                 // no scrolling
    nullptr,               // generic draw
    nullptr,               // default nav is fine
    selectOutletDeleteAllMenu,
    MENU_OUTLET_SCHEDULE
  },

// MENU_OUTLET_DELETE_CONFIRM
  {
    "DELETE OUTLET CONFIRM",
    nullptr,                 // no static item list
    0,
    false,
    [](){ drawConfirmDialog("DELETE OUTLET"); }, // drawFn
    handleConfirmMenu,       // navFn
    selectOutletDeleteConfirmMenu,
//    MENU_OUTLET_DELETE       // back target if needed
    MENU_OUTLET_SCHEDULE
},

  // MENU_OUTLET_DELETE_ALL_CONFIRM
  {
    "OUTLET DEL ALL CONF",
    nullptr,
    0,
    false,
    [](){ drawConfirmDialog("DELETE ALL OUTLET"); },
    handleConfirmMenu,
    selectOutletDeleteAllConfirmMenu,
    MENU_OUTLET_SCHEDULE
  },

  // MENU_MANUAL_DOSING
  {
    "MANUAL DOSING",
    manualDosingMenu,
    manualDosingMenuCount,
    true,
    nullptr,
    nullptr,
    selectManualDosingMenu,
    MENU_MAIN
  },

  // MENU_MANUAL_SELECT_PUMP
  MENU_PLACEHOLDER("MANUAL SELECT PUMP", MENU_MANUAL_DOSING),

  // MENU_MANUAL_SET_AMOUNT
  MENU_PLACEHOLDER("MANUAL SET AMOUNT", MENU_MANUAL_SELECT_PUMP),

  // MENU_MANUAL_CONFIRM
  MENU_PLACEHOLDER("MANUAL CONFIRM", MENU_MANUAL_DOSING),

  // MENU_PUMP_CALIBRATION
  {
    "PUMP CALIBRATION",
    pumpCalibrationMenu,
    pumpCalibrationMenuCount,
    true,
    nullptr,
    nullptr,
    selectPumpCalibrationMenu,
    MENU_MAIN
  },

  // MENU_CALIBRATE_P1
  { "CAL P1", nullptr, 0, false, nullptr, handleCalibrateMenu, nullptr, MENU_PUMP_CALIBRATION },
  // MENU_CALIBRATE_P1_START
  MENU_PLACEHOLDER("CAL P1 START", MENU_CALIBRATE_P1),
  // MENU_CALIBRATE_P1_CONFIRM
  MENU_PLACEHOLDER("CAL P1 CONFIRM", MENU_CALIBRATE_P1),

  // MENU_CALIBRATE_P2
  { "CAL P2", nullptr, 0, false, nullptr, handleCalibrateMenu, nullptr, MENU_PUMP_CALIBRATION },
  // MENU_CALIBRATE_P2_START
  MENU_PLACEHOLDER("CAL P2 START", MENU_CALIBRATE_P2),
  // MENU_CALIBRATE_P2_CONFIRM
  MENU_PLACEHOLDER("CAL P2 CONFIRM", MENU_CALIBRATE_P2),

  // MENU_CALIBRATE_P3
  { "CAL P3", nullptr, 0, false, nullptr, handleCalibrateMenu, nullptr, MENU_PUMP_CALIBRATION },
  // MENU_CALIBRATE_P3_START
  MENU_PLACEHOLDER("CAL P3 START", MENU_CALIBRATE_P3),
  // MENU_CALIBRATE_P3_CONFIRM
  MENU_PLACEHOLDER("CAL P3 CONFIRM", MENU_CALIBRATE_P3),

  // MENU_CALIBRATE_P4
  { "CAL P4", nullptr, 0, false, nullptr, handleCalibrateMenu, nullptr, MENU_PUMP_CALIBRATION },
  // MENU_CALIBRATE_P4_START
  MENU_PLACEHOLDER("CAL P4 START", MENU_CALIBRATE_P4),
  // MENU_CALIBRATE_P4_CONFIRM
  MENU_PLACEHOLDER("CAL P4 CONFIRM", MENU_CALIBRATE_P4),

  // MENU_TOPUP_SOLUTION
  {
    "TOP-UP SOLUTION",
    topupMenu,
    topupMenuCount,
    true,
    nullptr,
    nullptr,
    selectTopUpMenu,
    MENU_MAIN
  },

  // MENU_TOPUP_SET_AMOUNTS
  MENU_PLACEHOLDER("TOPUP AMOUNTS", MENU_TOPUP_SOLUTION),
  // MENU_TOPUP_AMOUNTS_CONFIRM
  MENU_PLACEHOLDER("TOPUP CONFIRM", MENU_TOPUP_SET_AMOUNTS),
  // MENU_TOPUP_SET_PUMP_PIN
  MENU_PLACEHOLDER("TOPUP PUMP PIN", MENU_TOPUP_SOLUTION),
  // MENU_TOPUP_PUMP_CONFIRM
  MENU_PLACEHOLDER("TOPUP PIN CONFIRM", MENU_TOPUP_SET_PUMP_PIN),

  // MENU_REPLACE_SOLUTION
  {
    "REPLACE SOLUTION",
    replaceMenu,
    replaceMenuCount,
    true,
    nullptr,
    nullptr,
    selectReplaceMenu,
    MENU_MAIN
  },

  // MENU_REPLACE_SET_AMOUNTS
  MENU_PLACEHOLDER("REPLACE AMOUNTS", MENU_REPLACE_SOLUTION),
  // MENU_REPLACE_SET_DRAIN
  MENU_PLACEHOLDER("REPLACE DRAIN", MENU_REPLACE_SOLUTION),
  // MENU_REPLACE_SET_FILL
  MENU_PLACEHOLDER("REPLACE FILL", MENU_REPLACE_SOLUTION),
  // MENU_REPLACE_SET_SCHEDULE
  MENU_PLACEHOLDER("REPLACE SCHEDULE", MENU_REPLACE_SOLUTION),
  // MENU_REPLACE_CONFIRM
  MENU_PLACEHOLDER("REPLACE CONFIRM", MENU_REPLACE_SOLUTION),

  // MENU_RESET_WIFI
  MENU_PLACEHOLDER("RESET WIFI", MENU_MAIN),
  // MENU_RESET_WIFI_CONFIRM
  MENU_PLACEHOLDER("RESET WIFI CONFIRM", MENU_MAIN),

  // MENU_FACTORY_RESET
  MENU_PLACEHOLDER("FACTORY RESET", MENU_MAIN),
  // MENU_FACTORY_RESET_CONFIRM
  MENU_PLACEHOLDER("FACTORY RESET CONF", MENU_MAIN),
  // MENU_DOSING_ERROR_NO_DAYS
// {
//   "ERROR: SELECT DAYS",
//   dosingErrorNoDaysMenu,
//   dosingErrorNoDaysMenuCount,
//   false,                    // no scrolling needed, only one item
//   nullptr,                  // generic draw
//   nullptr,                  // default nav
//   selectDosingErrorNoDaysMenu,
//   MENU_DOSING_ADD           // back target
// },

};
