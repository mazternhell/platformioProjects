// SimpleWiFi.h
// Simple WiFi management without WiFiManager dependency
// Handles AP mode for setup and Station mode for normal operation

#ifndef SIMPLEWIFI_H
#define SIMPLEWIFI_H

#include <Arduino.h>
#include <WiFi.h>

// WiFi connection states
enum class WiFiState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    AP_MODE,
    FAILED
};

// Initialize WiFi based on stored credentials
// If no credentials or connection fails, starts AP mode
void initWiFi();

// Attempt to connect to WiFi using stored credentials
// Returns true if connected, false otherwise
bool connectToWiFi();

// Start Access Point mode for initial setup
void startAPMode(bool persistFlag = false);

// Stop AP mode and switch to station mode
void stopAPMode();

// Check WiFi connection status and handle reconnection
void handleWiFi();

// Get current WiFi state
WiFiState getWiFiState();

// Get WiFi status as string for display
String getWiFiStatusString();

// Save new WiFi credentials and attempt connection
// Called from web interface
bool saveWiFiCredentials(const char* ssid, const char* password);

// Get current IP address (either AP or Station)
String getIPAddress();

// Check if in AP mode
bool isAPMode();

#endif // SIMPLEWIFI_H
