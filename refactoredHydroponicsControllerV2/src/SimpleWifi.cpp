// SimpleWiFi.cpp
// Simple WiFi management implementation

#include "SimpleWiFi.h"
#include "Globals.h"
#include <Preferences.h>
#include <esp_task_wdt.h>

// Local preferences object for WiFi credentials
static Preferences wifiPrefs;

// WiFi configuration
const char* AP_SSID = "Hydroponics-Setup";
const char* AP_PASSWORD = "hydro123";  // Minimum 8 chars for WPA2
const IPAddress AP_IP(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

// Connection parameters
const uint32_t WIFI_CONNECT_TIMEOUT = 15000;  // 15 seconds
// WIFI_RECONNECT_INTERVAL is defined in Globals.h
const uint8_t MAX_CONNECT_ATTEMPTS = 3;

// State tracking
static WiFiState currentState = WiFiState::DISCONNECTED;
static unsigned long lastReconnectAttempt = 0;
static uint8_t reconnectAttempts = 0;


// void initWiFi() {
//     Serial.println(F("\n=== Initializing WiFi ==="));

//     WiFi.setAutoReconnect(true);
//     WiFi.persistent(true);
    
//     // Load AP mode flag
//     wifiPrefs.begin("wifi", true);
//     config.wifi_ap_mode = wifiPrefs.getBool("ap_mode", false);
//     wifiPrefs.end();
    
//     // If AP mode forced (user selected AP-only), skip STA
//     if (config.wifi_ap_mode) {
//         Serial.println(F("AP mode flag set - starting AP mode"));
//         startAPMode();
//         return;
//     }

//     Serial.println(F("Attempting to connect to stored WiFi..."));
    
//     // Use stored credentials from NVS
//     WiFi.mode(WIFI_STA);
//     WiFi.begin();   // 🔥 THIS IS THE KEY FIX

//     unsigned long startAttempt = millis();
//     while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
//         delay(200);
//         Serial.print(F("."));
//     }
//     Serial.println();

//     if (WiFi.status() == WL_CONNECTED) {
//         Serial.print(F("Connected to WiFi: "));
//         Serial.println(WiFi.SSID());
//         currentState = WiFiState::CONNECTED;
//     } else {
//         Serial.println(F("WiFi failed - starting AP mode"));
//         currentState = WiFiState::AP_MODE;
//         startAPMode();
//     }
// }

void initWiFi() {
    Serial.println(F("\n=== Initializing WiFi ==="));

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    // Read ap_mode flag, but default to FALSE (only true if user explicitly forced AP)
    wifiPrefs.begin("wifi", true);
    config.wifi_ap_mode = wifiPrefs.getBool("ap_mode", false);
    wifiPrefs.end();

    // If user forced AP mode, honor it
    if (config.wifi_ap_mode) {
        Serial.println(F("AP mode flag set (forced) - starting AP mode"));
        startAPMode(false);
        return;
    }

    // Always try STA using stored NVS credentials
    Serial.println(F("Attempting to connect using stored WiFi credentials..."));
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // <-- uses last saved SSID/pass from NVS

    currentState = WiFiState::CONNECTING;
    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
        millis() - startTime < WIFI_CONNECT_TIMEOUT) {
        delay(250);
        Serial.print(".");
        esp_task_wdt_reset();
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        currentState = WiFiState::CONNECTED;
        reconnectAttempts = 0;

        String ssid = WiFi.SSID();
        strncpy(config.wifi_ssid, ssid.c_str(), sizeof(config.wifi_ssid) - 1);
        config.wifi_ssid[sizeof(config.wifi_ssid) - 1] = '\0';

        Serial.println(F("WiFi connected from stored creds!"));
        Serial.print(F("SSID: ")); Serial.println(config.wifi_ssid);
        Serial.print(F("IP: "));   Serial.println(WiFi.localIP());

        // Make sure forced AP flag is cleared if it was ever set
        wifiPrefs.begin("wifi", false);
        wifiPrefs.putBool("ap_mode", false);
        wifiPrefs.end();

    } else {
        currentState = WiFiState::FAILED;
        Serial.println(F("WiFi connection failed - starting AP mode TEMPORARILY"));
        
        // IMPORTANT: do NOT persist ap_mode=true on failure
        startAPMode(false);
    }
}

bool connectToWiFi() {
    // ESP32 will use its stored credentials automatically
    // Just need to call WiFi.begin() without parameters
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // Uses stored credentials
    
    Serial.print(F("Connecting to stored WiFi"));
    
    currentState = WiFiState::CONNECTING;
    unsigned long startTime = millis();
    
    // Wait for connection with timeout
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECT_TIMEOUT) {
        delay(250);
        Serial.print(".");
        esp_task_wdt_reset();  // Reset watchdog
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        currentState = WiFiState::CONNECTED;
        reconnectAttempts = 0;
        Serial.println(F("WiFi connected!"));
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());
        
        // Update config SSID for display
        String ssid = WiFi.SSID();
        strncpy(config.wifi_ssid, ssid.c_str(), sizeof(config.wifi_ssid) - 1);
        config.wifi_ssid[sizeof(config.wifi_ssid) - 1] = '\0';
        
        // Clear AP mode flag
        if (config.wifi_ap_mode) {
            config.wifi_ap_mode = false;
            wifiPrefs.begin("wifi", false);
            wifiPrefs.putBool("ap_mode", false);
            wifiPrefs.end();
        }
        
        return true;
    } else {
        currentState = WiFiState::FAILED;
        Serial.println(F("WiFi connection failed"));
        reconnectAttempts++;
        return false;
    }
}

// void startAPMode(bool persistFlag = false) {
//     Serial.println(F("\n=== Starting Access Point Mode ==="));
    
//     // Stop any existing WiFi connections
//     WiFi.disconnect();
//     delay(100);
    
//     // Configure AP
//     WiFi.mode(WIFI_AP);
//     WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    
//     bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
    
//     if (apStarted) {
//         currentState = WiFiState::AP_MODE;
//         Serial.println(F("Access Point started successfully"));
//         Serial.print(F("AP SSID: "));
//         Serial.println(AP_SSID);
//         Serial.print(F("AP Password: "));
//         Serial.println(AP_PASSWORD);
//         Serial.print(F("AP IP address: "));
//         Serial.println(WiFi.softAPIP());
//         Serial.println(F("Connect to the AP and navigate to http://192.168.4.1"));
        
//         // Update config
//         config.wifi_ap_mode = true;
//         // Save to EEPROM
//         wifiPrefs.begin("wifi", false);
//         wifiPrefs.putBool("ap_mode", true);
//         wifiPrefs.end();
//     } else {
//         currentState = WiFiState::FAILED;
//         Serial.println(F("Failed to start Access Point!"));
//     }
// }
void startAPMode(bool persistFlag) {
    Serial.println(F("\n=== Starting Access Point Mode ==="));

    WiFi.disconnect();
    delay(100);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

    if (apStarted) {
        currentState = WiFiState::AP_MODE;

        Serial.println(F("Access Point started successfully"));
        Serial.print(F("AP SSID: ")); Serial.println(AP_SSID);
        Serial.print(F("AP Password: ")); Serial.println(AP_PASSWORD);
        Serial.print(F("AP IP address: ")); Serial.println(WiFi.softAPIP());
        Serial.println(F("Connect to the AP and navigate to http://192.168.4.1"));

        // Runtime state: we ARE in AP now
        config.wifi_ap_mode = true;

        // Persist ONLY if user explicitly forced AP-only mode
        if (persistFlag) {
            wifiPrefs.begin("wifi", false);
            wifiPrefs.putBool("ap_mode", true);
            wifiPrefs.end();
            Serial.println(F("AP mode persisted (forced by user)."));
        } else {
            Serial.println(F("AP mode is temporary (not persisted)."));
        }
    } else {
        currentState = WiFiState::FAILED;
        Serial.println(F("Failed to start Access Point!"));
    }
}

void stopAPMode() {
    if (currentState == WiFiState::AP_MODE) {
        Serial.println(F("Stopping Access Point mode"));
        WiFi.softAPdisconnect(true);
        delay(100);
    }
}

void handleWiFi() {
    // If in AP mode, nothing to handle
    if (currentState == WiFiState::AP_MODE) {
        return;
    }
    
    // Check if we're still connected
    if (WiFi.status() != WL_CONNECTED && currentState == WiFiState::CONNECTED) {
        Serial.println(F("WiFi connection lost!"));
        currentState = WiFiState::DISCONNECTED;
        reconnectAttempts = 0;
    }
    
    // Attempt reconnection if disconnected
    if (currentState == WiFiState::DISCONNECTED || currentState == WiFiState::FAILED) {
        unsigned long now = millis();
        
        // Try reconnecting every WIFI_RECONNECT_INTERVAL
        if (now - lastReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            
            if (reconnectAttempts < MAX_CONNECT_ATTEMPTS) {
                Serial.print(F("Reconnection attempt "));
                Serial.print(reconnectAttempts + 1);
                Serial.print(F("/"));
                Serial.println(MAX_CONNECT_ATTEMPTS);
                
                connectToWiFi();
            } else {
                // Max attempts reached, fall back to AP mode
                Serial.println(F("Max reconnection attempts reached - switching to AP mode"));
                startAPMode(false);
                reconnectAttempts = 0;  // Reset for next time
            }
        }
    }
}

WiFiState getWiFiState() {
    return currentState;
}

String getWiFiStatusString() {
    switch (currentState) {
        case WiFiState::CONNECTED:
            return "Connected to " + String(config.wifi_ssid);
        case WiFiState::CONNECTING:
            return "Connecting...";
        case WiFiState::AP_MODE:
            return "AP: " + String(AP_SSID);
        case WiFiState::DISCONNECTED:
            return "Disconnected";
        case WiFiState::FAILED:
            return "Connection Failed";
        default:
            return "Unknown";
    }
}

bool saveWiFiCredentials(const char* ssid, const char* password) {
    if (ssid == nullptr || strlen(ssid) == 0) {
        Serial.println(F("Invalid SSID"));
        return false;
    }
    
    // Validate SSID length (max 32 chars)
    if (strlen(ssid) > 32) {
        Serial.println(F("SSID too long (max 32 chars)"));
        return false;
    }
    
    // Validate password length (0 or 8-63 chars for WPA2)
    if (password != nullptr && strlen(password) > 0) {
        if (strlen(password) < 8 || strlen(password) > 63) {
            Serial.println(F("Password must be 8-63 chars or empty for open network"));
            return false;
        }
    }
    
    // Update config for display purposes only
    strncpy(config.wifi_ssid, ssid, sizeof(config.wifi_ssid) - 1);
    config.wifi_ssid[sizeof(config.wifi_ssid) - 1] = '\0';
    
    if (password != nullptr) {
        strncpy(config.wifi_password, password, sizeof(config.wifi_password) - 1);
        config.wifi_password[sizeof(config.wifi_password) - 1] = '\0';
    } else {
        config.wifi_password[0] = '\0';
    }
    
    config.wifi_ap_mode = false;  // Will attempt station mode
    
    // Save credentials to ESP32's NVS using WiFi.begin()

    // This automatically stores them with WiFi.persistent(true)
    WiFi.persistent(true);          // make sure persistence is on
    WiFi.disconnect(true, true);    // clear old creds from RAM
    delay(100);
    // Keep AP up while starting STA
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);

    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
    
    // Save ONLY the ap_mode flag to Preferences
    wifiPrefs.begin("wifi", false);
    wifiPrefs.putBool("ap_mode", false);
    wifiPrefs.end();
    
    Serial.println(F("WiFi credentials saved to ESP32 NVS"));
    Serial.print(F("SSID: "));
    Serial.println(config.wifi_ssid);
    
    return true;
}

String getIPAddress() {
    if (currentState == WiFiState::AP_MODE) {
        return WiFi.softAPIP().toString();
    } else if (currentState == WiFiState::CONNECTED) {
        return WiFi.localIP().toString();
    } else {
        return "0.0.0.0";
    }
}

bool isAPMode() {
    return currentState == WiFiState::AP_MODE;
}
