/*
 * Storage.cpp
 * 
 * Implementation of file system and configuration storage functions.
 */

#include "Storage.h"

// ==================================================
// LITTLEFS INITIALIZATION
// ==================================================
bool initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    spiffsReady = false;
    return false;
  }

  size_t totalBytes = LittleFS.totalBytes();
  size_t usedBytes = LittleFS.usedBytes();

  spiffsReady = true;
  return true;
}

// ==================================================
// CONFIG MANAGEMENT
// ==================================================
void setDefaultConfig() {
  config.apPassword = "hydro2024";
  config.mqttBroker = "broker.hivemq.com";
  config.mqttPort = 1883;
  config.mqttUser = "";
  config.mqttPass = "";
  config.mqttTopic = "hydro";
  config.mqttSubTopic1 = "";
  config.mqttSubTopic2 = "";
  config.mqttSubTopic3 = "";
  config.publishInterval = 5000;
  config.enableLogging = true;
  config.mqttUseTLS = true;
  config.webUsername = WEB_USERNAME;
  config.webPassword = WEB_PASSWORD;
}

bool loadConfigFromLittleFS() {
  if (!spiffsReady) return false;

  if (!LittleFS.exists(CONFIG_FILE)) {
    return false;
  }

  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    return false;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("   JSON parse error: ");
    Serial.println(error.c_str());
    return false;
  }

  config.apPassword = doc["apPassword"] | "hydro2024";
  config.mqttBroker = doc["mqttBroker"] | "broker.hivemq.com";
  config.mqttPort = doc["mqttPort"] | 1883;
  config.mqttUser = doc["mqttUser"] | "";
  config.mqttPass = doc["mqttPass"] | "";
  config.mqttTopic = doc["mqttTopic"] | "hydro";
  config.mqttSubTopic1 = doc["mqttSubTopic1"] | "";
  config.mqttSubTopic2 = doc["mqttSubTopic2"] | "";
  config.mqttSubTopic3 = doc["mqttSubTopic3"] | "";
  config.publishInterval = doc["publishInterval"] | 5000;
  config.enableLogging = doc["enableLogging"] | true;

  if (doc.containsKey("mqttUseTLS")) {
    config.mqttUseTLS = doc["mqttUseTLS"];
  } else {
    config.mqttUseTLS = (config.mqttPort == 8883);
  }

  config.webUsername = doc["webUsername"] | WEB_USERNAME;
  config.webPassword = doc["webPassword"] | WEB_PASSWORD;

  return true;
}

bool saveConfigToLittleFS() {
  if (!spiffsReady) return false;

  StaticJsonDocument<1024> doc;

  doc["apPassword"] = config.apPassword;
  doc["mqttBroker"] = config.mqttBroker;
  doc["mqttPort"] = config.mqttPort;
  doc["mqttUser"] = config.mqttUser;
  doc["mqttPass"] = config.mqttPass;
  doc["mqttTopic"] = config.mqttTopic;
  doc["mqttSubTopic1"] = config.mqttSubTopic1;
  doc["mqttSubTopic2"] = config.mqttSubTopic2;
  doc["mqttSubTopic3"] = config.mqttSubTopic3;
  doc["publishInterval"] = config.publishInterval;
  doc["enableLogging"] = config.enableLogging;
  doc["mqttUseTLS"] = config.mqttUseTLS;
  doc["webUsername"] = config.webUsername;
  doc["webPassword"] = config.webPassword;

  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) {
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    file.close();
    return false;
  }

  file.close();
  return true;
}

// ==================================================
// PREFERENCES (EEPROM) STORAGE
// ==================================================
void loadSchedulesFromStorage() {
  preferences.begin("schedules", true);

  dosingScheduleCount = preferences.getInt("dose_count", 0);
  outletScheduleCount = preferences.getInt("outlet_count", 0);

  for (int i = 0; i < MAX_DOSING_SCHEDULES; i++) {
    String key = "dose_" + String(i);
    size_t len = preferences.getBytesLength(key.c_str());
    if (len == sizeof(DosingSchedule)) {
      preferences.getBytes(key.c_str(), &dosingSchedules[i], sizeof(DosingSchedule));
    }
  }

  for (int i = 0; i < MAX_OUTLET_SCHEDULES; i++) {
    String key = "outlet_" + String(i);
    size_t len = preferences.getBytesLength(key.c_str());
    if (len == sizeof(OutletSchedule)) {
      preferences.getBytes(key.c_str(), &outletSchedules[i], sizeof(OutletSchedule));
    }
  }

  preferences.end();
}

void saveSchedulesToStorage() {
  preferences.begin("schedules", false);

  preferences.putInt("dose_count", dosingScheduleCount);
  preferences.putInt("outlet_count", outletScheduleCount);

  for (int i = 0; i < MAX_DOSING_SCHEDULES; i++) {
    String key = "dose_" + String(i);
    preferences.putBytes(key.c_str(), &dosingSchedules[i], sizeof(DosingSchedule));
  }

  for (int i = 0; i < MAX_OUTLET_SCHEDULES; i++) {
    String key = "outlet_" + String(i);
    preferences.putBytes(key.c_str(), &outletSchedules[i], sizeof(OutletSchedule));
  }

  preferences.end();
}

void loadPumpCalibrationsFromStorage() {
  preferences.begin("pumps", true);

  for (int i = 0; i < 4; i++) {
    String key = "cal_" + String(i);
    size_t len = preferences.getBytesLength(key.c_str());
    if (len == sizeof(PumpCalibration)) {
      preferences.getBytes(key.c_str(), &pumpCalibrations[i], sizeof(PumpCalibration));
    }
  }

  preferences.end();
}

void savePumpCalibrationsToStorage() {
  preferences.begin("pumps", false);

  for (int i = 0; i < 4; i++) {
    String key = "cal_" + String(i);
    preferences.putBytes(key.c_str(), &pumpCalibrations[i], sizeof(PumpCalibration));
  }

  preferences.end();
}

void loadTopUpConfigFromStorage() {
  preferences.begin("topup", true);
  size_t len = preferences.getBytesLength("config");
  if (len == sizeof(TopUpConfig)) {
    preferences.getBytes("config", &topUpConfig, sizeof(TopUpConfig));
  }
  preferences.end();
}

void saveTopUpConfigToStorage() {
  preferences.begin("topup", false);
  preferences.putBytes("config", &topUpConfig, sizeof(TopUpConfig));
  preferences.end();
}

void loadReplaceConfigFromStorage() {
  preferences.begin("replace", true);
  size_t len = preferences.getBytesLength("config");
  if (len == sizeof(ReplaceConfig)) {
    preferences.getBytes("config", &replaceConfig, sizeof(ReplaceConfig));
  }
  preferences.end();
}

void saveReplaceConfigToStorage() {
  preferences.begin("replace", false);
  preferences.putBytes("config", &replaceConfig, sizeof(ReplaceConfig));
  preferences.end();
}

// ==================================================
// LOGGING
// ==================================================
void logSensorData() {
  if (!spiffsReady) return;

  DateTime now = rtc.now();
  float temp = rtc.getTemperature();

  File file = LittleFS.open(LOG_FILE, "a");
  if (!file) {
    return;
  }

  char logEntry[100];
  sprintf(logEntry, "%04d-%02d-%02d %02d:%02d:%02d,%.1f\n",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second(),
          temp);

  file.print(logEntry);
  file.close();

  // Keep log file manageable
  File checkFile = LittleFS.open(LOG_FILE, "r");
  if (checkFile && checkFile.size() > 50000) {
    checkFile.close();
    LittleFS.remove(LOG_FILE);
  } else {
    checkFile.close();
  }
}
