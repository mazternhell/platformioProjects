/*
 * DisplayUI.h
 * 
 * TFT display rendering functions for menus, screens, and UI elements.
 */

#ifndef DISPLAYUI_H
#define DISPLAYUI_H

#include "Globals.h"
#include "MenuRegistry.h"


// ==================================================
// DISPLAY INITIALIZATION
// ==================================================
void initDisplay();

// ==================================================
// STATUS BAR
// ==================================================
void drawCircleIndicator(int x, int y, bool connected);
void drawStatusBar();
void updateStatusBar();

// ==================================================
// MENU RENDERING
// ==================================================
void drawMenu();
void drawMainMenu();
void drawGenericMenu(const char* title, const char* const* items, int itemCount, bool useScrolling);
void drawConfirmDialog(const char* title);
void showSplash(const char* msg, uint16_t color = YELLOW, uint16_t ms = 900);

// ==================================================
// SCHEDULE SCREENS
// ==================================================
void drawDaySelectionScreen();
void drawDosingScheduleListScreen();
void drawDosingDeleteListScreen();
void drawScheduleEditorScreen();
void drawOutletEditorScreen();
void drawTimeSelectionScreen();
void drawAmountSelectionScreen();
void drawOutletScheduleListScreen();


// ==================================================
// CALIBRATION SCREEN
// ==================================================
void drawCalibrateMenu(uint8_t pumpNum);

#endif // DISPLAYUI_H
