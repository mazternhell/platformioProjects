// MenuRegistry.h
#ifndef MENU_REGISTRY_H
#define MENU_REGISTRY_H

#include "Globals.h"

// Function pointer types for menu behavior
typedef void (*MenuDrawFn)();
typedef void (*MenuSelectFn)();
typedef void (*MenuNavFn)();

struct MenuDef {
  const char* title;                 // Title string for generic menus
  const char* const* items;          // PROGMEM item array (nullptr for custom screens)
  int itemCount;                     // number of items (0 for custom screens)
  bool useScrolling;                 // enable scrollOffset logic

  MenuDrawFn   drawFn;               // nullptr => use generic draw
  MenuNavFn    navFn;                // nullptr => use default nav
  MenuSelectFn selectFn;             // nullptr => use legacy select fallback

  MenuState backMenu;                // where BACK goes (we'll wire later)
};

// MENUS[] indexed by MenuState enum order
extern const MenuDef MENUS[];

#endif
