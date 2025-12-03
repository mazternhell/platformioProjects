/*
 * WebServer.cpp
 * 
 * Implementation of web server and API endpoints.
 */

#include "WebServer.h"
#include "Storage.h"
#include "Hardware.h"

// Forward declarations for functions from main.cpp
void connectMQTT();
void resetNTPSync();
void startNTPSync();

// Forward declarations for SimpleWiFi functions
bool saveWiFiCredentials(const char* ssid, const char* password);
bool isAPMode();
String getIPAddress();
String getWiFiStatusString();

bool authenticate(AsyncWebServerRequest *request) {
  if (!request->authenticate(config.webUsername.c_str(), config.webPassword.c_str())) {
    return false;
  }
  return true;
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    client->text(getSensorDataJSON());
  } else if (type == WS_EVT_DISCONNECT) {
  }
}

void notifyWebClients() {
  ws.textAll(getSensorDataJSON());
}

void updateSensorData() {
  DateTime now = rtc.now();
  char timeStr[30];
  sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second());

  currentData.timestamp = String(timeStr);
  currentData.temperature = rtc.getTemperature();
  currentData.wifiStatus = wifiConnected;
  currentData.mqttStatus = mqttConnected;
  currentData.spiffsUsed = LittleFS.usedBytes() / 1024;
  currentData.spiffsTotal = LittleFS.totalBytes() / 1024;
  currentData.ip = WiFi.localIP().toString();
}

String getSensorDataJSON() {
  StaticJsonDocument<512> doc;

  doc["timestamp"] = currentData.timestamp;
  doc["temperature"] = currentData.temperature;
  doc["wifi"] = currentData.wifiStatus;
  doc["mqtt"] = currentData.mqttStatus;
  doc["spiffs"]["used"] = currentData.spiffsUsed;
  doc["spiffs"]["total"] = currentData.spiffsTotal;
  doc["ip"] = currentData.ip;
  doc["uptime"] = millis() / 1000;
  doc["ntpSynced"] = ntpSynced;

  String output;
  serializeJson(doc, output);
  return output;
}

String getConfigJSON() {
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
  doc["webUsername"] = config.webUsername;

  String output;
  serializeJson(doc, output);
  return output;
}

bool updateConfigFromJSON(String json) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    return false;
  }

  if (doc.containsKey("apPassword")) config.apPassword = doc["apPassword"].as<String>();
  if (doc.containsKey("mqttBroker")) config.mqttBroker = doc["mqttBroker"].as<String>();
  if (doc.containsKey("mqttPort")) config.mqttPort = doc["mqttPort"];
  if (doc.containsKey("mqttUser")) config.mqttUser = doc["mqttUser"].as<String>();
  if (doc.containsKey("mqttPass")) config.mqttPass = doc["mqttPass"].as<String>();
  if (doc.containsKey("mqttTopic")) config.mqttTopic = doc["mqttTopic"].as<String>();
  if (doc.containsKey("mqttSubTopic1")) config.mqttSubTopic1 = doc["mqttSubTopic1"].as<String>();
  if (doc.containsKey("mqttSubTopic2")) config.mqttSubTopic2 = doc["mqttSubTopic2"].as<String>();
  if (doc.containsKey("mqttSubTopic3")) config.mqttSubTopic3 = doc["mqttSubTopic3"].as<String>();
  if (doc.containsKey("publishInterval")) config.publishInterval = doc["publishInterval"];
  if (doc.containsKey("enableLogging")) config.enableLogging = doc["enableLogging"];
  if (doc.containsKey("webUsername")) config.webUsername = doc["webUsername"].as<String>();
  if (doc.containsKey("webPassword")) config.webPassword = doc["webPassword"].as<String>();

  bool saved = saveConfigToLittleFS();

  if (saved) {
    // Reconnect MQTT with new settings
    if (wifiConnected) {
      mqtt.disconnect();
      mqtt.setServer(config.mqttBroker.c_str(), config.mqttPort);
      connectMQTT();
    }
  }

  return saved;
}

void setupWebServer() {
  // WebSocket handler
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  // Serve main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getIndexHTML());
  });

  // API: Get current data
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", getSensorDataJSON());
  });

  // API: Get configuration
  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
    request->send(200, "application/json", getConfigJSON());
  });

  // API: Save configuration
  server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (index == 0) {
      // First chunk
    }

    String body = String((char*)data).substring(0, len);

    if (index + len == total) {
      // Last chunk - process the data
      if (updateConfigFromJSON(body)) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuration saved\"}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Failed to save configuration\"}");
      }
    }
  });

  // API: Download logs
  server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
    if (LittleFS.exists(LOG_FILE)) {
      request->send(LittleFS, LOG_FILE, "text/csv", true);
    } else {
      request->send(404, "text/plain", "No log file found");
    }
  });

  // API: Clear logs
  server.on("/api/logs/clear", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
    if (LittleFS.exists(LOG_FILE)) {
      LittleFS.remove(LOG_FILE);
      request->send(200, "application/json", "{\"success\":true,\"message\":\"Logs cleared\"}");
    } else {
      request->send(404, "application/json", "{\"success\":false,\"message\":\"No log file found\"}");
    }
  });

  // API: Reboot
  server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
    request->send(200, "application/json", "{\"success\":true,\"message\":\"Rebooting...\"}");
    delay(1000);
    ESP.restart();
  });

  // API: Sync NTP
  server.on("/api/sync-ntp", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
    if (wifiConnected) {
      resetNTPSync();
      startNTPSync();
      request->send(200, "application/json", "{\"success\":true,\"message\":\"NTP sync initiated\"}");
    } else {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"WiFi not connected\"}");
    }
  });

  // API: Get hardware state
  server.on("/api/hardware", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }
    request->send(200, "application/json", getHardwareJSON());
  });

  // API: Control relay
  server.on("/api/relay", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }

    if (request->hasParam("relay", true) && request->hasParam("state", true)) {
      uint8_t relay = request->getParam("relay", true)->value().toInt();
      bool state = (request->getParam("state", true)->value() == "true" ||
                    request->getParam("state", true)->value() == "1");

      if (relay >= 1 && relay <= 4) {
        setRelay(relay, state);
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid relay number\"}");
      }
    } else {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    }
  });

  // API: Control pump speed
  server.on("/api/pump", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }

    if (request->hasParam("pump", true) && request->hasParam("speed", true)) {
      uint8_t pump = request->getParam("pump", true)->value().toInt();
      uint8_t speed = request->getParam("speed", true)->value().toInt();

      if (pump >= 1 && pump <= 4 && speed <= 100) {
        setPumpSpeed(pump, speed);
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid pump or speed\"}");
      }
    } else {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    }
  });

  // API: Control LED indicator
  server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }

    if (request->hasParam("led", true) && request->hasParam("state", true)) {
      uint8_t led = request->getParam("led", true)->value().toInt();
      bool state = (request->getParam("state", true)->value() == "true" ||
                    request->getParam("state", true)->value() == "1");

      if (led >= 1 && led <= 4) {
        setLED(led, state);
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid LED number\"}");
      }
    } else {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    }
  });

  // API: Control WS2812B RGB LED
  server.on("/api/ws2812b", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!authenticate(request)) {
      return request->requestAuthentication();
    }

    if (request->hasParam("r", true) && request->hasParam("g", true) && request->hasParam("b", true)) {
      uint8_t r = request->getParam("r", true)->value().toInt();
      uint8_t g = request->getParam("g", true)->value().toInt();
      uint8_t b = request->getParam("b", true)->value().toInt();

      setWS2812B(r, g, b);
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing RGB parameters\"}");
    }
  });

  // API: Configure WiFi credentials (for AP mode setup)
  // server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
  //   if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
  //     String ssid = request->getParam("ssid", true)->value();
  //     String password = request->getParam("password", true)->value();
      
  //     if (saveWiFiCredentials(ssid.c_str(), password.c_str())) {
  //       request->send(200, "application/json", "{\"success\":true,\"message\":\"WiFi credentials saved. Please manually reboot the device.\"}");
  //     } else {
  //       request->send(400, "application/json", "{\"success\":false,\"message\":\"Failed to save WiFi credentials\"}");
  //     }
  //   } else {
  //     request->send(400, "application/json", "{\"success\":false,\"message\":\"Missing SSID or password\"}");
  //   }
  // });
server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("ssid", true)) {
        request->send(400, "application/json",
                      "{\"success\":false,\"message\":\"Missing SSID\"}");
        return;
    }

    String ssid = request->getParam("ssid", true)->value();
    String password = request->hasParam("password", true)
                        ? request->getParam("password", true)->value()
                        : "";

    if (saveWiFiCredentials(ssid.c_str(), password.c_str())) {
        request->send(200, "application/json",
                      "{\"success\":true,\"message\":\"WiFi credentials saved. Rebooting...\"}");
        // optional: schedule a reboot (see Fix D)
        scheduleRestart();
    } else {
        request->send(400, "application/json",
                      "{\"success\":false,\"message\":\"Failed to save WiFi credentials\"}");
    }
});

  // API: Get WiFi status
  server.on("/api/wifi/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    doc["apMode"] = isAPMode();
    doc["connected"] = (WiFi.status() == WL_CONNECTED);
    doc["ssid"] = isAPMode() ? "Hydroponics-Setup" : String(config.wifi_ssid);
    doc["ip"] = getIPAddress();
    doc["status"] = getWiFiStatusString();
    
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  // 404 handler
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });
}