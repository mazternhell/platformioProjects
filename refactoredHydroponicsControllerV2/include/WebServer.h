/*
 * WebServer.h
 * 
 * Web server setup and API endpoints for remote monitoring and control.
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

// Define HTTP method constants BEFORE including ESPAsyncWebServer
// (normally from WebServer.h, but we can't include that due to conflicts)
#ifndef HTTP_ANY
#define HTTP_ANY     0b01111111
#define HTTP_GET     0b00000001
#define HTTP_POST    0b00000010
#define HTTP_PUT     0b00000100
#define HTTP_PATCH   0b00001000
#define HTTP_DELETE  0b00010000
#define HTTP_OPTIONS 0b00100000
#endif

#include "Globals.h"
#include "WebPages.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

// ==================================================
// WEB SERVER SETUP
// ==================================================
void setupWebServer();

// ==================================================
// WEBSOCKET COMMUNICATION
// ==================================================
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len);
void notifyWebClients();
void updateSensorData();

// ==================================================
// AUTHENTICATION
// ==================================================
bool authenticate(AsyncWebServerRequest *request);

// ==================================================
// JSON API RESPONSES
// ==================================================
String getSensorDataJSON();
String getConfigJSON();
bool updateConfigFromJSON(String json);

#endif // WEBSERVER_H
