/*
 * DisplayUI.cpp
 * 
 * Implementation of TFT display rendering functions.
 */

#include "DisplayUI.h"

#define DARKGREY 0x7BEF   // or any grey shade you like

void showSplash(const char* msg, uint16_t color, uint16_t ms) {
  // Simple centered toast-style message
  tft.fillRect(0, 52, 128, 24, BLACK);
  tft.setTextSize(1);
  tft.setTextColor(color);
  
  // crude centering: each char ~6px at size 1
  int len = strlen(msg);
  int x = max(0, (128 - len * 6) / 2);
  tft.setCursor(x, 60);
  tft.print(msg);

  delay(ms);

  // force menu to redraw after toast
  menuNav.needsFullRedraw = true;
  menuNav.needsRedraw = true;
  menuNav.lastDrawnIndex = -1;
}

void drawCircleIndicator(int x, int y, bool connected) {
  // Light green when connected (0x07E0), red when disconnected
  uint16_t color = connected ? 0x07E0 : RED;
  // Draw filled circle - 7 pixels tall (same as text height)
  // Center point at x+3, y+3
  tft.fillRect(x+1, y, 5, 7, color);      // Main vertical bar
  tft.fillRect(x, y+1, 7, 5, color);      // Main horizontal bar
  tft.drawPixel(x+1, y+1, color);         // Round top-left
  tft.drawPixel(x+5, y+1, color);         // Round top-right
  tft.drawPixel(x+1, y+5, color);         // Round bottom-left
  tft.drawPixel(x+5, y+5, color);         // Round bottom-right
}

// Draw status bar with date, time, and connection status

void drawStatusBar() {
  // Month names
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

  // Get current time
  DateTime now = rtc.now();

  // ====== TOP LINE - Date, Day, Time (evenly distributed) ======
  // Layout: "01/15"  "MON"  "14:30:45"
  // Positions: 2px, 46px, 80px (user adjusted)

  tft.setTextSize(1);
  tft.setTextColor(GREEN);  // ALL GREEN

  // Date: "01/15" at x=2
  tft.setCursor(2, 2);
  if (now.month() < 10) tft.print("0");
  tft.print(now.month());
  tft.print("/");
  if (now.day() < 10) tft.print("0");
  tft.print(now.day());

  // Day: "MON" at x=46 (user adjusted)
  tft.setCursor(46, 2);
  // Convert to uppercase
  const char* dayName = days[now.dayOfTheWeek()];
  for (int i = 0; dayName[i] != '\0'; i++) {
    tft.print((char)toupper(dayName[i]));
  }

  // Time: "14:30:45" at x=80 (user adjusted)
  tft.setCursor(80, 2);
  if (now.hour() < 10) tft.print("0");
  tft.print(now.hour());
  tft.print(":");
  if (now.minute() < 10) tft.print("0");
  tft.print(now.minute());
  tft.print(":");
  if (now.second() < 10) tft.print("0");
  tft.print(now.second());

  // ====== SECOND LINE - WiFi, MQTT, Test LED, RTC Sync ======
  // Layout: "WiFi ●  MQTT ● ●      14:25"
  // Positions: x=2, x=30, x=42, x=68, x=76, x=98
  // WiFi text(2) + circle(30) | MQTT text(42) + circle(68) | Test circle(76) | Sync(98)

  tft.setTextColor(GREEN);  // ALL GREEN

  // WiFi status at x=2
  tft.setCursor(2, 10);
  tft.print("WiFi");
  drawCircleIndicator(30, 10, wifiConnected);  // Circle at x=30

  // MQTT status at x=42
  tft.setCursor(42, 10);
  tft.print("MQTT");
  drawCircleIndicator(68, 10, mqtt.connected());  // Circle at x=68

  // Test LED indicator at x=76 (no text, just circle beside MQTT)
  drawCircleIndicator(76, 10, testLedState);  // GREEN when ON, RED when OFF

  // Last RTC sync time at x=98
  tft.setCursor(98, 10);
  if (lastSyncHour < 10) tft.print("0");
  tft.print(lastSyncHour);
  tft.print(":");
  if (lastSyncMinute < 10) tft.print("0");
  tft.print(lastSyncMinute);

  // No horizontal line - just blank space separator
}

// Update status bar periodically (non-blocking, selective updates only)

void updateStatusBar() {
  unsigned long currentTime = millis();

  // Only update if on main menu and enough time has passed
  if (menuNav.currentMenu != MENU_MAIN) return;
  if (currentTime - lastStatusBarUpdate < STATUS_BAR_UPDATE_INTERVAL) return;

  lastStatusBarUpdate = currentTime;

  // Month and day names
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

  // Get current time
  DateTime now = rtc.now();

  tft.setTextSize(1);
  tft.setTextColor(GREEN);  // ALL GREEN

  // ====== UPDATE ONLY CHANGED ELEMENTS ======

  // Update date if day changed (x=2, y=2)
  if (now.day() != lastDisplayedDay) {
    tft.fillRect(2, 2, 36, 8, BLACK);  // Clear wider area to prevent artifacts
    tft.setTextSize(1);  // Ensure correct text size
    tft.setTextColor(GREEN);  // Ensure correct color
    tft.setCursor(2, 2);  // Always set cursor explicitly
    if (now.month() < 10) tft.print("0");
    tft.print(now.month());
    tft.print("/");
    if (now.day() < 10) tft.print("0");
    tft.print(now.day());
    lastDisplayedDay = now.day();

    // Also update day of week (x=46, y=2) - UPPERCASE - USER ADJUSTED
    tft.fillRect(46, 2, 24, 8, BLACK);  // Clear wider area to prevent artifacts
    tft.setCursor(46, 2);
    const char* dayName = days[now.dayOfTheWeek()];
    for (int i = 0; dayName[i] != '\0'; i++) {
      tft.print((char)toupper(dayName[i]));
    }
  }

  // Update time - only seconds change frequently (x=80, y=2) - USER ADJUSTED
  bool timeChanged = false;
  if (now.hour() != lastDisplayedHour || now.minute() != lastDisplayedMinute || now.second() != lastDisplayedSecond) {
    tft.fillRect(80, 2, 48, 8, BLACK);  // Clear to edge to prevent artifacts
    tft.setTextSize(1);  // Ensure correct text size
    tft.setTextColor(GREEN);  // Ensure correct color
    tft.setCursor(80, 2);  // Always set cursor explicitly
    if (now.hour() < 10) tft.print("0");
    tft.print(now.hour());
    tft.print(":");
    if (now.minute() < 10) tft.print("0");
    tft.print(now.minute());
    tft.print(":");
    if (now.second() < 10) tft.print("0");
    tft.print(now.second());

    lastDisplayedHour = now.hour();
    lastDisplayedMinute = now.minute();
    lastDisplayedSecond = now.second();
  }

  // Update WiFi circle if state changed (x=30, y=10)
  if (wifiConnected != lastWifiState) {
    tft.fillRect(28, 10, 10, 8, BLACK);  // Clear wider area including circle
    drawCircleIndicator(30, 10, wifiConnected);
    lastWifiState = wifiConnected;
  }

  // Update MQTT circle if state changed (x=68, y=10) - moved closer to WiFi
  bool mqttState = mqtt.connected();
  if (mqttState != lastMqttState) {
    tft.fillRect(66, 10, 10, 8, BLACK);  // Clear wider area including circle
    drawCircleIndicator(68, 10, mqttState);
    lastMqttState = mqttState;
  }

  // Sync time rarely changes, so only update when it actually changes
  // Check if we need to update sync time display
  static uint8_t displayedSyncHour = 255;
  static uint8_t displayedSyncMinute = 255;
  if (lastSyncHour != displayedSyncHour || lastSyncMinute != displayedSyncMinute) {
    // Sync time changed, update display
    tft.fillRect(98, 10, 30, 8, BLACK);  // Clear sync time area
    tft.setCursor(98, 10);
    tft.setTextColor(GREEN);
    if (lastSyncHour < 10) tft.print("0");
    tft.print(lastSyncHour);
    tft.print(":");
    if (lastSyncMinute < 10) tft.print("0");
    tft.print(lastSyncMinute);
    displayedSyncHour = lastSyncHour;
    displayedSyncMinute = lastSyncMinute;
  }

  // Update test LED indicator if state changed (x=76, y=10) - beside MQTT
  static bool lastTestLedState = false;
  if (testLedState != lastTestLedState) {
    tft.fillRect(74, 10, 10, 8, BLACK);  // Clear test LED circle area
    drawCircleIndicator(76, 10, testLedState);
    lastTestLedState = testLedState;
  }
}

void drawGenericMenu(const char* title,
                     const char* const* items,
                     int itemCount,
                     bool useScrolling,
                     int cursorX,
                     int textX,
                     int startY) 
{
  // Draw title only on full redraw
  if (menuNav.lastDrawnIndex == -1) {
    tft.setTextSize(1);
    tft.setCursor(2, 2);
    tft.setTextColor(YELLOW);
    tft.print(title);
    tft.drawFastHLine(0, 10, 128, YELLOW);
  }

  int visibleItems = useScrolling ? MENU_ITEMS_PER_PAGE : itemCount;

  for (int i = 0; i < visibleItems && (i + menuNav.scrollOffset) < itemCount; i++) {
    int itemIndex = useScrolling ? (i + menuNav.scrollOffset) : i;
    int y = startY + (i * MENU_ITEM_HEIGHT);

    bool isSelected  = (itemIndex == menuNav.selectedIndex);
    bool wasSelected = (itemIndex == menuNav.lastDrawnIndex);

    if (isSelected != wasSelected || menuNav.lastDrawnIndex == -1) {

      // Clear row
      tft.fillRect(0, y - 1, 128, MENU_ITEM_HEIGHT, BLACK);

      // Arrow + color
      if (isSelected) {
        tft.setTextColor(WHITE);
        tft.setCursor(cursorX, y);
        tft.print(">");
      } else {
        tft.setTextColor(GREEN);
      }

      // Read text from PROGMEM
      char buffer[50];
      strcpy_P(buffer, (char*)pgm_read_ptr(&items[itemIndex]));

      // Print item text
      tft.setCursor(textX, y);
      tft.print(buffer);
    }
  }
}

// Legacy drawer (your existing switch-based behavior)
static void drawMenuLegacy(MenuState menu) {
  switch (menu) {
    case MENU_MAIN:
      drawMainMenu(); break;
    case MENU_SCHEDULING:
      drawGenericMenu("SCHEDULING", schedulingMenuItems, schedulingMenuCount, true, 2, 12, 15); break;
    case MENU_DOSING_SCHEDULE:
      drawGenericMenu("DOSING SCHEDULE", dosingScheduleMenu, dosingScheduleMenuCount, true, 2, 12, 15); break;
    case MENU_DOSING_VIEW:
      drawDosingScheduleListScreen(); break;
    case MENU_DOSING_DELETE:
      drawDosingDeleteListScreen(); break;
    case MENU_DOSING_DELETE_CONFIRM:
      drawConfirmDialog("   DELETE SCHEDULE?"); break;
    case MENU_MANUAL_DOSING:
      drawGenericMenu("MANUAL DOSING", manualDosingMenu, manualDosingMenuCount, true, 2, 12, 15); break;
    case MENU_OUTLET_SCHEDULE:
      drawGenericMenu("OUTLET SCHEDULE", outletScheduleMenu, outletScheduleMenuCount, true, 2, 12, 15); break;
    case MENU_OUTLET_VIEW:
      drawOutletScheduleListScreen(); break;  
    case MENU_OUTLET_ADD:
      drawOutletEditorScreen(); break;
    case MENU_OUTLET_ADD_SELECT_DAYS:
      drawDaySelectionScreen(); break;
    case MENU_PUMP_CALIBRATION:
      drawGenericMenu("PUMP CALIBRATION", pumpCalibrationMenu, pumpCalibrationMenuCount, true, 2, 12, 15); break;
    case MENU_TOPUP_SOLUTION:
      drawGenericMenu("TOP-UP SOLUTION", topupMenu, topupMenuCount, true, 2, 12, 15); break;
    case MENU_REPLACE_SOLUTION:
      drawGenericMenu("REPLACE SOLUTION", replaceMenu, replaceMenuCount, true, 2, 12, 15); break;
    case MENU_DOSING_DELETE_ALL:
      drawConfirmDialog("DELETE ALL DOSING"); break;
    case MENU_OUTLET_DELETE_ALL:
      drawConfirmDialog("DELETE ALL OUTLET"); break;
    case MENU_DOSING_ADD:
      drawScheduleEditorScreen(); break;
    case MENU_DOSING_ADD_SELECT_DAYS:
      drawDaySelectionScreen(); break;
    case MENU_DOSING_ADD_SET_TIME:
      drawTimeSelectionScreen(); break;
    case MENU_DOSING_ADD_SET_AMOUNT:
      drawAmountSelectionScreen(); break;
    case MENU_RESET_WIFI_CONFIRM:
      drawConfirmDialog("RESET WIFI"); break;
    case MENU_FACTORY_RESET_CONFIRM:
      drawConfirmDialog("FACTORY RESET"); break;
    case MENU_CALIBRATE_P1:
      drawCalibrateMenu(1); break;
    case MENU_CALIBRATE_P2:
      drawCalibrateMenu(2); break;
    case MENU_CALIBRATE_P3:
      drawCalibrateMenu(3); break;
    case MENU_CALIBRATE_P4:
      drawCalibrateMenu(4); break;
    default:
      tft.setCursor(2, 2);
      tft.setTextColor(YELLOW);
      tft.setTextSize(1);
      tft.print("MENU ");
      tft.print(menuNav.currentMenu);
      tft.setCursor(2, 40);
      tft.setTextColor(WHITE);
      tft.print("Screen pending");
      tft.setCursor(2, 60);
      tft.setTextColor(CYAN);
      tft.print("Press BACK");
      break;
  }
}

void drawMenu() {
  if (!menuNav.needsRedraw) return;

  auto drawViaRegistryOrLegacy = [](){
    const MenuDef& m = MENUS[menuNav.currentMenu];

    // If registry knows how to draw, use it
    if (m.drawFn) {
      m.drawFn();
      return;
    }
    if (m.items && m.itemCount > 0) {
      // Standard generic menu layout for now
      drawGenericMenu(m.title, m.items, m.itemCount, m.useScrolling, 2, 12, 15);
      return;
    }

    // Otherwise fallback to legacy switch
    drawMenuLegacy(menuNav.currentMenu);
  };

  // Full redraw
  if (menuNav.needsFullRedraw) {
    menuNav.needsFullRedraw = false;
    menuNav.needsRedraw = false;
    menuNav.lastDrawnIndex = -1;
    tft.fillScreen(BLACK);

    drawViaRegistryOrLegacy();
    menuNav.lastDrawnIndex = menuNav.selectedIndex;
    return;
  }

  // Partial redraw checks (keep your existing behavior)
  bool needsPartialRedraw = false;
  if (menuNav.currentMenu == MENU_DOSING_ADD_SELECT_DAYS || menuNav.currentMenu == MENU_OUTLET_ADD_SELECT_DAYS) {
    static int lastDayIndex = -1;
    if (lastDayIndex != menuNav.daySelectIndex) {
      needsPartialRedraw = true;
      lastDayIndex = menuNav.daySelectIndex;
    }
  } else {
    if (menuNav.lastDrawnIndex != menuNav.selectedIndex || menuNav.needsRedraw) {
      needsPartialRedraw = true;
    }
  }

  if (needsPartialRedraw) {
    menuNav.needsRedraw = false;
    drawViaRegistryOrLegacy();
    menuNav.lastDrawnIndex = menuNav.selectedIndex;
  }
}

void drawDaySelectionScreen() {
  static int lastDayIndex = -1;
  static uint8_t lastDaysBitmap = 0xFF;  // Initialize to impossible value

  // Full redraw on first draw or if bitmap changed significantly
  bool fullRedraw = (menuNav.lastDrawnIndex == -1) || (lastDaysBitmap != menuNav.tempDaysBitmap);

  if (fullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);

    // Header
    tft.setTextColor(YELLOW);
    tft.setCursor(2, 2);
    tft.print("SELECT DAYS");
    tft.drawFastHLine(0, 10, 128, YELLOW);

    lastDayIndex = -1;  // Force redraw of all items
    lastDaysBitmap = menuNav.tempDaysBitmap;
  }

  int y = 15;
  int itemHeight = 11;

  // Draw all 7 days - but only redraw changed items
  for (int i = 0; i < 7; i++) {
    bool selected = (i == menuNav.daySelectIndex);
    bool wasSelected = (i == lastDayIndex);
    bool checked = isDayEnabled(menuNav.tempDaysBitmap, i);

    // Only redraw if selection changed or full redraw needed
    if (fullRedraw || selected != wasSelected) {
      int itemY = y + (i * itemHeight);

      // Clear the line
      tft.fillRect(0, itemY - 1, 128, itemHeight, BLACK);

      // Highlight selected item
      if (selected) {
        tft.fillRect(0, itemY - 1, 128, itemHeight, BLUE);
        tft.setTextColor(WHITE);
      } else {
        tft.setTextColor(GREEN);
      }

      tft.setCursor(4, itemY + 1);
      // Checkbox
      tft.print(checked ? "[X]" : "[ ]");
      tft.print(" ");
      tft.print(dayNames[i]);
    }
  }

  // Done button (index 7)
  int doneY = y + (7 * itemHeight) + 4;
  bool doneSelected = (menuNav.daySelectIndex == 7);
  bool doneWasSelected = (lastDayIndex == 7);

  if (fullRedraw || doneSelected != doneWasSelected) {
    // Clear the line
    tft.fillRect(0, doneY - 1, 128, itemHeight, BLACK);

    if (doneSelected) {
      tft.fillRect(0, doneY - 1, 128, itemHeight, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(4, doneY + 1);
    tft.print("> Done");
  }

  lastDayIndex = menuNav.daySelectIndex;
}


void drawDosingScheduleListScreen() {
  tft.fillScreen(BLACK);
  tft.setTextSize(1);

  const int SCHEDULES_PER_PAGE = 6;
  const int LINE_HEIGHT = 9;  // Each text line is 9px (6px char + 1px spacing + 2px gap)
  
  // Show message if no schedules
  if (dosingScheduleCount == 0) {
    tft.setTextColor(CYAN);
    tft.setCursor(4, 50);
    tft.print("No schedules");
    tft.setCursor(4, 62);
    tft.print("Press to Return");
    return;
  }

  // Calculate which schedules to show on current page
  int startIdx = menuNav.currentPage * SCHEDULES_PER_PAGE;
  int endIdx = min(startIdx + SCHEDULES_PER_PAGE, dosingScheduleCount);
  
  int y = 0;  // Start at top of screen
  
  // Draw schedules (2 lines each)
  for (int i = startIdx; i < endIdx; i++) {
    DosingSchedule &sched = dosingSchedules[i];
    
    // LINE 1: "S1 Pump1 08:30 5.0mL"
    tft.setCursor(0, y);
    tft.setTextColor(GREEN);
    tft.print("S");
    tft.print(i + 1);  // Schedule number (1-indexed)
    tft.print(" Pump");
    tft.print(sched.pumpNumber);
    tft.print(" ");
    if (sched.hour < 10) tft.print("0");
    tft.print(sched.hour);
    tft.print(":");
    if (sched.minute < 10) tft.print("0");
    tft.print(sched.minute);
    tft.print(" ");
    tft.print(sched.amountML / 10.0, 1);
    tft.print("mL");
    
    y += LINE_HEIGHT;
    
    // LINE 2: Day abbreviations with color coding
    const char* dayAbbr[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    tft.setCursor(0, y);
    
    for (int d = 0; d < 7; d++) {
      bool enabled = isDayEnabled(sched.daysOfWeek, d);
      
      // Set color based on enabled state (no brackets)
      if (enabled) {
        tft.setTextColor(GREEN);
      } else {
        tft.setTextColor(RED);
      }
      
      tft.print(dayAbbr[d]);
      
      if (d < 6) tft.print(" ");  // Space between days
    }
    
    y += LINE_HEIGHT + 2;  // Add 2px gap between schedules
  }
  
  // "Press to Return" at y=119
  tft.setCursor(0, 119);
  tft.setTextColor(YELLOW);
  tft.print("Press to Return");
}


void drawDosingDeleteListScreen() {
  static int lastSelectedIndex = -1;
  static int lastPage = -1;

  // Check if we need full redraw
  bool needsFullRedraw = (menuNav.needsFullRedraw || lastPage != menuNav.currentPage);
  
  // Full redraw if page changed or first draw
  if (needsFullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    lastSelectedIndex = -1;
    lastPage = menuNav.currentPage;
    menuNav.needsFullRedraw = false;  // Clear flag immediately
  }

  // Show message if no schedules
  if (dosingScheduleCount == 0) {
    tft.setTextColor(CYAN);
    tft.setCursor(4, 50);
    tft.print("No schedules");
    tft.setCursor(4, 62);
    tft.print("Press to Return");
    return;
  }

  const int SCHEDULES_PER_PAGE = 6;
  const int LINE_HEIGHT = 20;  // 2 lines per schedule (9px + 9px + 2px gap)
  
  // Calculate which schedules to show on current page
  int startIdx = menuNav.currentPage * SCHEDULES_PER_PAGE;
  int endIdx = min(startIdx + SCHEDULES_PER_PAGE, dosingScheduleCount);
  
  int y = 0;  // Start at top of screen
  int itemsOnPage = 0;
  
  // Draw schedules (2 lines each, 20px total per schedule)
  for (int i = startIdx; i < endIdx; i++) {
    bool isSelected = (i == menuNav.selectedIndex);
    bool wasSelected = (i == lastSelectedIndex);
    
    // Redraw if selection changed, full redraw, or first draw
    if (isSelected != wasSelected || needsFullRedraw || lastSelectedIndex == -1) {
      DosingSchedule &sched = dosingSchedules[i];
      
      // Clear 2-line area (20px total)
      tft.fillRect(0, y, 128, LINE_HEIGHT, isSelected ? BLUE : BLACK);
      
      // Set text color
      tft.setTextColor(isSelected ? WHITE : GREEN);
      tft.setTextSize(1);
      
      // LINE 1: "S1 Pump1 08:30 5.0mL"
      tft.setCursor(0, y);
      tft.print("S");
      tft.print(i + 1);
      tft.print(" Pump");
      tft.print(sched.pumpNumber);
      tft.print(" ");
      if (sched.hour < 10) tft.print("0");
      tft.print(sched.hour);
      tft.print(":");
      if (sched.minute < 10) tft.print("0");
      tft.print(sched.minute);
      tft.print(" ");
      tft.print(sched.amountML / 10.0, 1);
      tft.print("mL");
      
      // LINE 2: Day abbreviations with color coding
      const char* dayAbbr[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
      tft.setCursor(0, y + 9);
      
      for (int d = 0; d < 7; d++) {
        bool enabled = isDayEnabled(sched.daysOfWeek, d);
        
        // Set color based on enabled and selection state
        if (isSelected) {
          tft.setTextColor(enabled ? WHITE : RED);
        } else {
          tft.setTextColor(enabled ? GREEN : RED);
        }
        
        tft.print(dayAbbr[d]);
        if (d < 6) tft.print(" ");
      }
    }
    
    y += LINE_HEIGHT;  // Move to next schedule
    itemsOnPage++;
  }
  
  // Draw "Return to Dosing Menu" option if on the page where last schedule is
  int returnIndex = dosingScheduleCount;
  int returnPage = returnIndex / SCHEDULES_PER_PAGE;
  
  if (menuNav.currentPage == returnPage && itemsOnPage < SCHEDULES_PER_PAGE) {
    bool isSelected = (menuNav.selectedIndex == returnIndex);
    bool wasSelected = (lastSelectedIndex == returnIndex);
    
    if (isSelected != wasSelected || needsFullRedraw || lastSelectedIndex == -1) {
      // Clear line area
      tft.fillRect(0, y, 128, 10, isSelected ? BLUE : BLACK);
      
      // Set text color
      tft.setTextColor(isSelected ? WHITE : YELLOW);
      tft.setTextSize(1);
      tft.setCursor(0, y);
      tft.print("Return to Menu");
    }
  }
  
  lastSelectedIndex = menuNav.selectedIndex;
}


void drawScheduleEditorScreen() {
  static int lastSelectedIndex = -1;
  static uint8_t lastDaysBitmap = 0xFF;
  static uint8_t lastPump = 0;
  static uint8_t lastHour = 0;
  static uint8_t lastMinute = 0;
  static uint16_t lastAmount = 0;
  static bool lastInEditMode = false;
  static bool lastEditingHour = false;

  bool fullRedraw = (menuNav.lastDrawnIndex == -1);

  if (fullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);

    // Header
    tft.setTextColor(YELLOW);
    tft.setCursor(2, 2);
    tft.print(" ADD DOSING SCHEDULE ");
    tft.drawFastHLine(0, 10, 128, YELLOW);

    lastSelectedIndex = -1;  // Force redraw all
  }

  int y = 15;
  int lineHeight = 14;

  // Line 0: Pump
  if (fullRedraw || lastSelectedIndex == 0 || menuNav.selectedIndex == 0 || lastPump != tempDosingSchedule.pumpNumber) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    if (menuNav.selectedIndex == 0) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(4, y + 2);
    tft.print("Pump: ");

    if (menuNav.selectedIndex == 0 && menuNav.inEditMode) {
      tft.setTextColor(YELLOW);
    }

    tft.print(tempDosingSchedule.pumpNumber);

  }
  y += lineHeight;

  // Line 1: Days (2 lines allocated)
  if (fullRedraw || lastSelectedIndex == 1 || menuNav.selectedIndex == 1 || lastDaysBitmap != menuNav.tempDaysBitmap) {
    tft.fillRect(0, y, 128, lineHeight * 2, BLACK);
  // Line 1: Days
  bool daysEditing = (menuNav.selectedIndex == 1 && menuNav.inEditMode);

  if (!daysEditing && menuNav.selectedIndex == 1) {
      // Normal selection highlight (not editing)
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(BLACK);
  } else {
      // Editing OR not selected → black background
      tft.fillRect(0, y, 128, lineHeight, BLACK);
      tft.setTextColor(WHITE);
  }
    tft.setCursor(4, y + 2);
    tft.print("Days:");

    // // Print days on second line with word wrap
    // Print days on second line
    tft.setCursor(4, y + 2 + lineHeight);

    if (menuNav.selectedIndex == 1 && menuNav.inEditMode) {
      // Inline edit view: color-coded days + yellow cursor
      const char* dayAbbr[7] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

      for (int d = 0; d < 7; d++) {
        bool enabled  = (menuNav.tempDaysBitmap & (1 << d));
        bool cursorOn = (menuNav.daySelectIndex == d);

        if (cursorOn) {
          tft.setTextColor(YELLOW);          // cursor highlight
        } else {
          tft.setTextColor(enabled ? GREEN   // enabled day
                                  : RED);   // disabled day
        }

        tft.print(dayAbbr[d]);
        if (d < 6) tft.print(" ");
      }

      // DONE slot at the end
      tft.print(" ");
      if (menuNav.daySelectIndex == 7) tft.setTextColor(YELLOW);
      else tft.setTextColor(GREEN);
      tft.print("DONE");

      // restore a sane default for later prints
      tft.setTextColor(WHITE);
    } 
    else {
      // Normal compact view
      tft.setTextColor(WHITE);
      String daysStr = formatDaysCompact(menuNav.tempDaysBitmap);
      tft.print(daysStr);
    }

  }
  y += lineHeight * 2;

  // Line 2: Time
  if (fullRedraw || lastSelectedIndex == 2 || menuNav.selectedIndex == 2 ||
      lastHour != tempDosingSchedule.hour || lastMinute != tempDosingSchedule.minute ||
      lastInEditMode != menuNav.inEditMode || lastEditingHour != menuNav.editingHour) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    if (menuNav.selectedIndex == 2) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(4, y + 2);
    tft.print("Time: ");

    // Show edit indicator with color (same font size)
    if (menuNav.selectedIndex == 2 && menuNav.inEditMode) {
      if (menuNav.editingHour) {
        tft.setTextColor(YELLOW);
        if (tempDosingSchedule.hour < 10) tft.print("0");
        tft.print(tempDosingSchedule.hour);
        tft.setTextColor(WHITE);
        tft.print(":");
        if (tempDosingSchedule.minute < 10) tft.print("0");
        tft.print(tempDosingSchedule.minute);
      } else {
        if (tempDosingSchedule.hour < 10) tft.print("0");
        tft.print(tempDosingSchedule.hour);
        tft.print(":");
        tft.setTextColor(YELLOW);
        if (tempDosingSchedule.minute < 10) tft.print("0");
        tft.print(tempDosingSchedule.minute);
      }
    } else {
      if (tempDosingSchedule.hour < 10) tft.print("0");
      tft.print(tempDosingSchedule.hour);
      tft.print(":");
      if (tempDosingSchedule.minute < 10) tft.print("0");
      tft.print(tempDosingSchedule.minute);
    }
    lastHour = tempDosingSchedule.hour;
    lastMinute = tempDosingSchedule.minute;
  }
  y += lineHeight;

  // Line 3: Amount
  if (fullRedraw || lastSelectedIndex == 3 || menuNav.selectedIndex == 3 ||
      lastAmount != tempDosingSchedule.amountML || lastInEditMode != menuNav.inEditMode) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    if (menuNav.selectedIndex == 3) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(4, y + 2);
    tft.print("Amount: ");

    // Show edit indicator with color (same font size)
    if (menuNav.selectedIndex == 3 && menuNav.inEditMode) {
      tft.setTextColor(YELLOW);
    }
    tft.print(tempDosingSchedule.amountML / 10.0, 1);  // Show decimal
    tft.print(" mL");
    lastAmount = tempDosingSchedule.amountML;
  }
  y += lineHeight + 4;

  // Line 4: Save
  if (fullRedraw || lastSelectedIndex == 4 || menuNav.selectedIndex == 4) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    if (menuNav.selectedIndex == 4) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(4, y + 2);
    tft.print("[ Save ]");
  }
  y += lineHeight;

  // Line 5: Cancel
  if (fullRedraw || lastSelectedIndex == 5 || menuNav.selectedIndex == 5) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    if (menuNav.selectedIndex == 5) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(4, y + 2);
    tft.print("[ Cancel ]");
  }

  lastSelectedIndex = menuNav.selectedIndex;
  lastInEditMode = menuNav.inEditMode;
  lastEditingHour = menuNav.editingHour;
}

void drawOutletEditorScreen() {
  static int lastSelectedIndex = -1;
  static uint8_t lastDaysBitmap = 0xFF;
//  static uint8_t lastRelay = 0;
  static uint8_t lastRelay = 1;
  static uint8_t lastHourOn = 0, lastMinOn = 0, lastHourOff = 0, lastMinOff = 0;
  static bool lastInterval = false;
  static uint16_t lastIntervalMin = 0;
  static bool lastInEditMode = false;
  static bool lastEditingHour = false;
  static bool lastIntervalIsHours = false;
  static uint8_t lastIntervalValue = 0;

  bool fullRedraw = (menuNav.lastDrawnIndex == -1);

  if (fullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);

    tft.setTextColor(YELLOW);
    tft.setCursor(2, 2);
    tft.print(" ADD OUTLET SCHEDULE ");
    tft.drawFastHLine(0, 10, 128, YELLOW);

    lastSelectedIndex = -1;
  }

  int y = 15;
  int lineHeight = 14;

  auto highlightLine = [&](int idx, bool editing=false){
    if (menuNav.selectedIndex == idx) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(editing ? YELLOW : WHITE);
    } else {
      tft.fillRect(0, y, 128, lineHeight, BLACK);
      tft.setTextColor(GREEN);
    }
  };

  // 0) Relay
  if (fullRedraw || lastSelectedIndex==0 || menuNav.selectedIndex==0 || lastRelay!=tempOutletSchedule.relayNumber || lastInEditMode!=menuNav.inEditMode) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    highlightLine(0, menuNav.inEditMode);
    tft.setCursor(4, y+2);
    tft.print("Relay: ");
    tft.print(tempOutletSchedule.relayNumber);
  }
  y += lineHeight;

  // 1) Days (uses tempDaysBitmap like dosing)
  if (fullRedraw || lastSelectedIndex==1 || menuNav.selectedIndex==1 || lastDaysBitmap!=menuNav.tempDaysBitmap) {
    tft.fillRect(0, y, 128, lineHeight*2, BLACK);

    bool daysEditing = (menuNav.selectedIndex==1 && menuNav.inEditMode);
    if (!daysEditing && menuNav.selectedIndex==1) {
      tft.fillRect(0, y, 128, lineHeight, BLUE);
      tft.setTextColor(BLACK);
    } else {
      tft.fillRect(0, y, 128, lineHeight, BLACK);
      tft.setTextColor(WHITE);
    }

    tft.setCursor(4, y+2);
    tft.print("Days:");

    // second line: print enabled day abbreviations
  //   tft.setCursor(4, y+2+lineHeight);
  //   const char* dayAbbr[7] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
  //   for (int d=0; d<7; d++) {
  //     if (menuNav.tempDaysBitmap & (1<<d)) {
  //       tft.setTextColor(GREEN);
  //     } else {
  //       tft.setTextColor(RED);
  //     }
  //     tft.print(dayAbbr[d]);
  //     if (d<6) tft.print(" ");
  //   }
  // }
  // second line: print enabled day abbreviations + yellow cursor when editing
  tft.setCursor(4, y + 2 + lineHeight);
  const char* dayAbbr[7] = {"Su","Mo","Tu","We","Th","Fr","Sa"};

  if (menuNav.selectedIndex == 1 && menuNav.inEditMode) {
    // Inline edit view: color-coded days + yellow cursor + DONE
    for (int d = 0; d < 7; d++) {
      bool enabled  = (menuNav.tempDaysBitmap & (1 << d));
      bool cursorOn = (menuNav.daySelectIndex == d);

      if (cursorOn) {
        tft.setTextColor(YELLOW);
      } else {
        tft.setTextColor(enabled ? GREEN : RED);
      }

      tft.print(dayAbbr[d]);
      if (d < 6) tft.print(" ");
    }

    // DONE slot at the end
    tft.print(" ");
    tft.setTextColor(menuNav.daySelectIndex == 7 ? YELLOW : GREEN);
    tft.print("DONE");

    tft.setTextColor(WHITE);
  } else {
    // Normal compact view
    for (int d = 0; d < 7; d++) {
      bool enabled = (menuNav.tempDaysBitmap & (1 << d));
      tft.setTextColor(enabled ? GREEN : RED);
      tft.print(dayAbbr[d]);
      if (d < 6) tft.print(" ");
    }
  }
}
  
  y += lineHeight*2;

  // 2) Interval row
  if (fullRedraw || lastSelectedIndex==2 || menuNav.selectedIndex==2 ||
      lastInterval!=tempOutletSchedule.isInterval ||
      lastIntervalMin!=tempOutletSchedule.intervalMinutes ||
      lastIntervalIsHours!=menuNav.outletIntervalIsHours ||
      lastIntervalValue!=menuNav.outletIntervalValue ||
      lastInEditMode!=menuNav.inEditMode ||
      lastEditingHour!=menuNav.editingHour) {

    tft.fillRect(0, y, 128, lineHeight, BLACK);
    highlightLine(2, menuNav.inEditMode);

    tft.setCursor(4, y+2);
    tft.print("Interval: ");

    if (!tempOutletSchedule.isInterval) {
      tft.setTextColor(RED);
      tft.print("OFF");
    } else {
      tft.setTextColor(GREEN);
      // show unit + value from nav temps while editing, else from stored minutes
      bool showHours;
      uint8_t showVal;
      if (menuNav.selectedIndex==2 && menuNav.inEditMode) {
        showHours = menuNav.outletIntervalIsHours;
        showVal   = menuNav.outletIntervalValue;

      } else {
        showHours = (tempOutletSchedule.intervalMinutes >= 60);
        showVal   = showHours ? (tempOutletSchedule.intervalMinutes/60) : tempOutletSchedule.intervalMinutes;
      }
      tft.print(showVal);
      tft.print(showHours ? "h" : "m");
    }
  }
  y += lineHeight;

  // 3) Time ON (inactive if interval)
  if (fullRedraw || lastSelectedIndex==3 || menuNav.selectedIndex==3 ||
      lastHourOn!=tempOutletSchedule.hourOn || lastMinOn!=tempOutletSchedule.minuteOn ||
      lastInterval!=tempOutletSchedule.isInterval || lastInEditMode!=menuNav.inEditMode ||
      lastEditingHour!=menuNav.editingHour) {

    tft.fillRect(0, y, 128, lineHeight, BLACK);

    if (tempOutletSchedule.isInterval) {
      // greyed line
      tft.setTextColor(DARKGREY);
      tft.setCursor(4, y+2);
      tft.print("Time ON: --:--");
    } else {
      highlightLine(3, menuNav.inEditMode);
      tft.setCursor(4, y+2);
      tft.print("Time ON: ");
      if (menuNav.selectedIndex==3 && menuNav.inEditMode) tft.setTextColor(YELLOW);
      if (tempOutletSchedule.hourOn<10) tft.print("0");
      tft.print(tempOutletSchedule.hourOn);
      tft.print(":");
      if (tempOutletSchedule.minuteOn<10) tft.print("0");
      tft.print(tempOutletSchedule.minuteOn);
    }
  }
  y += lineHeight;

  // 4) Time OFF (inactive if interval)
  if (fullRedraw || lastSelectedIndex==4 || menuNav.selectedIndex==4 ||
      lastHourOff!=tempOutletSchedule.hourOff || lastMinOff!=tempOutletSchedule.minuteOff ||
      lastInterval!=tempOutletSchedule.isInterval || lastInEditMode!=menuNav.inEditMode ||
      lastEditingHour!=menuNav.editingHour) {

    tft.fillRect(0, y, 128, lineHeight, BLACK);

    if (tempOutletSchedule.isInterval) {
      tft.setTextColor(DARKGREY);
      tft.setCursor(4, y+2);
      tft.print("Time OFF: --:--");
    } else {
      highlightLine(4, menuNav.inEditMode);
      tft.setCursor(4, y+2);
      tft.print("Time OFF:");
      tft.setCursor(68, y+2);
      if (menuNav.selectedIndex==4 && menuNav.inEditMode) tft.setTextColor(YELLOW);
      if (tempOutletSchedule.hourOff<10) tft.print("0");
      tft.print(tempOutletSchedule.hourOff);
      tft.print(":");
      if (tempOutletSchedule.minuteOff<10) tft.print("0");
      tft.print(tempOutletSchedule.minuteOff);
    }
  }
  y += lineHeight;

  // 5) Save
  if (fullRedraw || lastSelectedIndex==5 || menuNav.selectedIndex==5) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    highlightLine(5);
    tft.setCursor(4, y+2);
    tft.print("SAVE");
  }
  y += lineHeight;

  // 6) Cancel
  if (fullRedraw || lastSelectedIndex==6 || menuNav.selectedIndex==6) {
    tft.fillRect(0, y, 128, lineHeight, BLACK);
    highlightLine(6);
    tft.setCursor(4, y+2);
    tft.print("CANCEL");
  }

  lastSelectedIndex = menuNav.selectedIndex;
  lastDaysBitmap = menuNav.tempDaysBitmap;
  lastRelay = tempOutletSchedule.relayNumber;
  lastHourOn = tempOutletSchedule.hourOn;
  lastMinOn = tempOutletSchedule.minuteOn;
  lastHourOff = tempOutletSchedule.hourOff;
  lastMinOff = tempOutletSchedule.minuteOff;
  lastInterval = tempOutletSchedule.isInterval;
  lastIntervalMin = tempOutletSchedule.intervalMinutes;
  lastInEditMode = menuNav.inEditMode;
  lastEditingHour = menuNav.editingHour;
  lastIntervalIsHours = menuNav.outletIntervalIsHours;
  lastIntervalValue = menuNav.outletIntervalValue;
}

void drawTimeSelectionScreen() {
  static uint8_t lastHour = 255;
  static uint8_t lastMinute = 255;
  static bool lastEditingHour = true;

  bool fullRedraw = (menuNav.lastDrawnIndex == -1);

  if (fullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);

    // Header
    tft.setTextColor(YELLOW);
    tft.setCursor(2, 2);
    tft.print("SET TIME");
    tft.drawFastHLine(0, 10, 128, YELLOW);

    // Instructions
    tft.setTextSize(1);
    tft.setTextColor(CYAN);
    int y = 90;
    tft.setCursor(2, y);
    tft.print("Rotate: Change");
    y += 10;
    tft.setCursor(2, y);
    tft.print("Click: Next field");

    lastHour = 255;  // Force redraw
    lastMinute = 255;
  }

  // Only redraw time if changed
  if (fullRedraw || lastHour != tempDosingSchedule.hour || lastEditingHour != menuNav.editingHour) {
    int y = 45;
    int xOffset = 20;

    // Clear hour area
    tft.fillRect(xOffset - 2, y - 2, 36, 26, BLACK);

    // Hour
    tft.setTextSize(3);
    if (menuNav.editingHour) {
      tft.fillRect(xOffset - 2, y - 2, 36, 26, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(xOffset, y);
    if (tempDosingSchedule.hour < 10) tft.print("0");
    tft.print(tempDosingSchedule.hour);

    lastHour = tempDosingSchedule.hour;
  }

  // Colon (only on full redraw)
  if (fullRedraw) {
    int y = 45;
    int xOffset = 20;
    tft.setTextSize(3);
    tft.setTextColor(CYAN);
    tft.setCursor(xOffset + 38, y);
    tft.print(":");
  }

  // Only redraw minute if changed
  if (fullRedraw || lastMinute != tempDosingSchedule.minute || lastEditingHour != menuNav.editingHour) {
    int y = 45;
    int xOffset = 20;

    // Clear minute area
    tft.fillRect(xOffset + 52, y - 2, 36, 26, BLACK);

    // Minute
    tft.setTextSize(3);
    if (!menuNav.editingHour) {
      tft.fillRect(xOffset + 52, y - 2, 36, 26, BLUE);
      tft.setTextColor(WHITE);
    } else {
      tft.setTextColor(GREEN);
    }
    tft.setCursor(xOffset + 54, y);
    if (tempDosingSchedule.minute < 10) tft.print("0");
    tft.print(tempDosingSchedule.minute);

    lastMinute = tempDosingSchedule.minute;
  }

  // Update editing status text
  if (fullRedraw || lastEditingHour != menuNav.editingHour) {
    int y = 110;
    tft.setTextSize(1);
    tft.fillRect(0, y, 128, 10, BLACK);  // Clear line
    tft.setTextColor(CYAN);
    tft.setCursor(2, y);
    if (menuNav.editingHour) {
      tft.print("Editing: Hour");
    } else {
      tft.print("Editing: Minute");
    }
    lastEditingHour = menuNav.editingHour;
  }
}


void drawAmountSelectionScreen() {
  static uint16_t lastAmount = 0xFFFF;

  bool fullRedraw = (menuNav.lastDrawnIndex == -1);

  if (fullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);

    // Header
    tft.setTextColor(YELLOW);
    tft.setCursor(2, 2);
    tft.print("SET AMOUNT");
    tft.drawFastHLine(0, 10, 128, YELLOW);

    // Instructions
    tft.setTextSize(1);
    tft.setTextColor(CYAN);
    int y = 100;
    tft.setCursor(2, y);
    tft.print("Rotate: Change (0.1mL)");
    y += 10;
    tft.setCursor(2, y);
    tft.print("Click: Confirm");

    lastAmount = 0xFFFF;  // Force redraw
  }

  // Only redraw amount if changed
  if (fullRedraw || lastAmount != tempDosingSchedule.amountML) {
    int y = 50;

    // Clear amount area
    tft.fillRect(20, y, 90, 26, BLACK);

    // Amount display
    tft.setTextSize(3);
    tft.setTextColor(GREEN);
    tft.setCursor(25, y);
    tft.print(tempDosingSchedule.amountML / 10.0, 1);
    tft.setTextSize(2);
    tft.setCursor(75, y + 8);
    tft.print(" mL");

    lastAmount = tempDosingSchedule.amountML;
  }
}
// outlet draw routines
void drawOutletScheduleListScreen() {
  static int lastSelectedIndex = -1;
  static int lastPage = -1;

  // bool needsFullRedraw = (menuNav.needsFullRedraw || lastPage != menuNav.currentPage);

  // if (needsFullRedraw) {
  //   tft.fillScreen(BLACK);
  //   tft.setTextSize(1);
  //   lastSelectedIndex = -1;
  //   lastPage = menuNav.currentPage;
  //   menuNav.needsFullRedraw = false;
  // }
    // ✅ ALSO full redraw on fresh entry (drawMenu clears needsFullRedraw early)
  bool needsFullRedraw = (menuNav.lastDrawnIndex == -1) || (lastPage != menuNav.currentPage);

  if (needsFullRedraw) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    lastSelectedIndex = -1;
    lastPage = menuNav.currentPage;
  }

  // No schedules
  if (outletScheduleCount == 0) {
    tft.setTextColor(CYAN);
    tft.setCursor(4, 50);
    tft.print("No schedules");
    tft.setCursor(4, 62);
    tft.print("Press to Return");
    return;
  }

  const int SCHEDULES_PER_PAGE = 6;
  const int LINE_HEIGHT = 20;

  // int startIdx = menuNav.currentPage * SCHEDULES_PER_PAGE;
  // int endIdx   = min(startIdx + SCHEDULES_PER_PAGE, outletScheduleCount);
  int startIdx = menuNav.currentPage * SCHEDULES_PER_PAGE;
  if (startIdx >= outletScheduleCount) {
    menuNav.currentPage = 0;
    menuNav.selectedIndex = 0;
    menuNav.needsFullRedraw = true;
    return;  // next frame redraws page 0
  }
  int endIdx = min(startIdx + SCHEDULES_PER_PAGE, outletScheduleCount);

  int y = 0;
  int itemsOnPage = 0;

  for (int i = startIdx; i < endIdx; i++) {
    bool isSelected  = (i == menuNav.selectedIndex);
    bool wasSelected = (i == lastSelectedIndex);

    if (isSelected != wasSelected || needsFullRedraw || lastSelectedIndex == -1) {
      OutletSchedule &sched = outletSchedules[i];

      tft.setTextColor(GREEN);
      tft.setTextSize(1);

      // // LINE 1: "S1 Rly3 08:00-09:30"
      // tft.setCursor(0, y);
      // tft.print("S");
      // tft.print(i + 1);
      // tft.print(" Rly");
      // tft.print(sched.relayNumber);
      // tft.print(" ");

      // if (sched.hourOn < 10) tft.print("0");
      // tft.print(sched.hourOn);
      // tft.print(":");
      // if (sched.minuteOn < 10) tft.print("0");
      // tft.print(sched.minuteOn);

      // tft.print("-");

      // if (sched.hourOff < 10) tft.print("0");
      // tft.print(sched.hourOff);
      // tft.print(":");
      // if (sched.minuteOff < 10) tft.print("0");
      // tft.print(sched.minuteOff);

    // LINE 1: either time span or interval
    tft.setCursor(0, y);
    tft.print("S");
    tft.print(i + 1);
    tft.print(" Rly");
    tft.print(sched.relayNumber);
    tft.print(" ");

    if (sched.isInterval) {
      tft.print("Int ");
      uint16_t mins = sched.intervalMinutes;

      if (mins % 60 == 0) {
        tft.print(mins / 60);
        tft.print("h");
      } else {
        tft.print(mins);
        tft.print("m");
      }
    } else {
      if (sched.hourOn < 10) tft.print("0");
      tft.print(sched.hourOn);
      tft.print(":");
      if (sched.minuteOn < 10) tft.print("0");
      tft.print(sched.minuteOn);

      tft.print("-");

      if (sched.hourOff < 10) tft.print("0");
      tft.print(sched.hourOff);
      tft.print(":");
      if (sched.minuteOff < 10) tft.print("0");
      tft.print(sched.minuteOff);
    }

      // LINE 2: days
      const char* dayAbbr[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
      tft.setCursor(0, y + 9);

      for (int d = 0; d < 7; d++) {
        bool enabled = isDayEnabled(sched.daysOfWeek, d);
        tft.setTextColor(enabled ? GREEN : RED);
        tft.print(dayAbbr[d]);
        if (d < 6) tft.print(" ");
      }
    }

    y += LINE_HEIGHT;
    itemsOnPage++;
  }

  // "Return to Menu" line at end (same pattern as dosing list)
  int returnIndex = outletScheduleCount;
  int returnPage = returnIndex / SCHEDULES_PER_PAGE;

  if (menuNav.currentPage == returnPage && itemsOnPage < SCHEDULES_PER_PAGE) {
    bool isSelected  = (menuNav.selectedIndex == returnIndex);
    bool wasSelected = (lastSelectedIndex == returnIndex);

    if (isSelected != wasSelected || needsFullRedraw || lastSelectedIndex == -1) {
      tft.fillRect(0, y, 128, 10, isSelected ? BLUE : BLACK);
      tft.setTextColor(isSelected ? WHITE : YELLOW);
      tft.setTextSize(1);
      tft.setCursor(0, y);
      tft.print("Return to Menu");
    }
  }

  lastSelectedIndex = menuNav.selectedIndex;
}


void drawMainMenu() {
  // Draw status bar only on full redraw
  if (menuNav.lastDrawnIndex == -1) {
    drawStatusBar();
    // No "MAIN MENU" text - cleaner look
    // No yellow line - blank space separator
    
    // Draw IP address at bottom (y=118)
    tft.setTextSize(1);
    tft.setTextColor(YELLOW);
    tft.setCursor(2, 118);
    if (wifiConnected && currentData.ip.length() > 0) {
      tft.print("IP: ");
      tft.print(currentData.ip);
    } else {
      tft.print("IP: N/A");
    }
  }

  // Start menu items below status bar
  int startY = 22;  // Back to 2 status lines

  for (int i = 0; i < MENU_ITEMS_PER_PAGE && (i + menuNav.scrollOffset) < mainMenuCount; i++) {
    int itemIndex = i + menuNav.scrollOffset;
    int y = startY + (i * MENU_ITEM_HEIGHT);

    // Only redraw if this item's selection state changed
    bool isSelected = (itemIndex == menuNav.selectedIndex);
    bool wasSelected = (itemIndex == menuNav.lastDrawnIndex);

    if (isSelected != wasSelected || menuNav.lastDrawnIndex == -1) {
      // Clear the line
      tft.fillRect(0, y - 1, 128, MENU_ITEM_HEIGHT, BLACK);

      if (isSelected) {
        tft.setTextColor(WHITE);
        // Draw arrow indicator
        tft.setCursor(2, y);
        tft.print(">");  // Arrow indicator
        tft.setCursor(12, y);  // Text starts after arrow
      } else {
        tft.setTextColor(GREEN);
        tft.setCursor(12, y);  // Text at same position (no arrow)
      }

      // Read string from PROGMEM
      char buffer[50];
      strcpy_P(buffer, (char*)pgm_read_ptr(&mainMenuItems[itemIndex]));
      tft.print(buffer);
    }
  }
}

// Draw dosing schedule menu

void drawConfirmDialog(const char* title) {
  static int lastConfirmIndex = -1;
  
  // Full redraw on first draw
  if (menuNav.needsFullRedraw || lastConfirmIndex == -1) {
    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    tft.setCursor(2, 2);
    tft.setTextColor(YELLOW);
    tft.print(title);
    tft.drawFastHLine(0, 10, 128, YELLOW);

    tft.setCursor(2, 40);
    tft.setTextColor(WHITE);
    tft.print("Are you sure?");
    
    lastConfirmIndex = -1;  // Force redraw of all options
    menuNav.needsFullRedraw = false;
  }

  int startY = 60;

  for (int i = 0; i < confirmYesNoMenuCount; i++) {
    int y = startY + (i * MENU_ITEM_HEIGHT);

    bool isSelected = (i == menuNav.selectedIndex);
    bool wasSelected = (i == lastConfirmIndex);

    if (isSelected != wasSelected || lastConfirmIndex == -1) {
      tft.fillRect(0, y - 1, 128, MENU_ITEM_HEIGHT, BLACK);

      if (isSelected) {
        tft.fillRect(0, y - 1, 128, MENU_ITEM_HEIGHT, BLUE);
        tft.setTextColor(WHITE);
      } else {
        tft.setTextColor(GREEN);
      }

      tft.setCursor(2, y);
      char buffer[10];
      strcpy_P(buffer, (char*)pgm_read_ptr(&confirmYesNoMenu[i]));
      tft.print(buffer);
    }
  }
  
  lastConfirmIndex = menuNav.selectedIndex;
}


void drawCalibrateMenu(uint8_t pumpNum) {
  if (menuNav.lastDrawnIndex == -1) {
    tft.setTextSize(1);
    tft.setCursor(2, 2);
    tft.setTextColor(YELLOW);
    tft.print("CALIBRATE PUMP ");
    tft.print(pumpNum);
    tft.drawFastHLine(0, 10, 128, YELLOW);
  }

  int startY = 40;

  for (int i = 0; i < calibrateConfirmMenuCount; i++) {
    int y = startY + (i * MENU_ITEM_HEIGHT);

    bool isSelected = (i == menuNav.selectedIndex);
    bool wasSelected = (i == menuNav.lastDrawnIndex);

    if (isSelected != wasSelected || menuNav.lastDrawnIndex == -1) {
      tft.fillRect(0, y - 1, 128, MENU_ITEM_HEIGHT, BLACK);

      if (isSelected) {
        tft.fillRect(0, y - 1, 128, MENU_ITEM_HEIGHT, BLUE);
        tft.setTextColor(WHITE);
      } else {
        tft.setTextColor(GREEN);
      }

      tft.setCursor(2, y);
      tft.print(calibrateConfirmMenu[i]);
    }
  }
}


void initDisplay() {
  // Aggressive protection against double initialization
  static bool initialized = false;
  if (initialized) {
    return;
  }

  // Clear the screen completely
  tft.fillScreen(BLACK);

  // Set text properties
  tft.setTextSize(1);
  tft.setTextWrap(false);

  // Mark as initialized
  initialized = true;
  displayInitialized = true;
}