/*
 * Storage.h
 * 
 * File system and configuration storage functions.
 * Handles LittleFS initialization, config files, schedule persistence,
 * and Preferences (EEPROM) storage.
 */

#ifndef STORAGE_H
#define STORAGE_H

#include "Globals.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// ==================================================
// LITTLEFS FUNCTIONS
// ==================================================
bool initLittleFS();

// ==================================================
// CONFIG MANAGEMENT
// ==================================================
void setDefaultConfig();
bool loadConfigFromLittleFS();
bool saveConfigToLittleFS();

// ==================================================
// PREFERENCES (EEPROM) STORAGE
// ==================================================
void loadSchedulesFromStorage();
void saveSchedulesToStorage();
void loadPumpCalibrationsFromStorage();
void savePumpCalibrationsToStorage();
void loadTopUpConfigFromStorage();
void saveTopUpConfigToStorage();
void loadReplaceConfigFromStorage();
void saveReplaceConfigToStorage();

// ==================================================
// LOGGING
// ==================================================
void logSensorData();

#endif // STORAGE_H
