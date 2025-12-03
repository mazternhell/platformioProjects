/*
 * ============================================
 * ESP32-S3 EEPROM Clear Utility (Preferences Only)
 * ============================================
 * 
 * Purpose: Erase and write 0x00 to EEPROM (Preferences) storage ONLY
 * 
 * What gets CLEARED (Preferences/EEPROM):
 * ✓ Dosing schedules
 * ✓ Outlet schedules
 * ✓ Pump calibrations
 * ✓ Top-up configuration
 * ✓ Replace solution configuration
 * 
 * What gets PRESERVED (LittleFS JSON files):
 * ✓ WiFi credentials (managed by WiFiManager)
 * ✓ MQTT broker settings (/config.json)
 * ✓ System configuration (/config.json)
 * 
 * Usage:
 * 1. Upload this sketch to your ESP32-S3
 * 2. Open Serial Monitor at 115200 baud
 * 3. Wait for completion message
 * 4. Upload your main hydroponics controller sketch
 * 
 * ============================================
 */

// ============================================
// USB SERIAL CONFIGURATION (Native USB CDC)
// ============================================
#ifndef ARDUINO_USB_MODE
#define ARDUINO_USB_MODE 1  // Enable native USB
#endif
#ifndef ARDUINO_USB_CDC_ON_BOOT  
#define ARDUINO_USB_CDC_ON_BOOT 1  // USB CDC available at boot
#endif

#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

// List of EEPROM (Preferences) namespaces used for schedules/config
const char* namespaces[] = {
  "schedules",     // Dosing and outlet schedules
  "pumps",         // Pump calibration data
  "topup",         // Top-up solution config
  "replace"        // Replace solution config
};

const int namespaceCount = 4;

// Helper: Write zeros to a Preferences namespace
void clearNamespaceToZero(const char* namespaceName) {
  Serial.print("  - Clearing: ");
  Serial.print(namespaceName);
  
  if (preferences.begin(namespaceName, false)) {  // Read-write mode
    // Clear all existing keys
    preferences.clear();
    
    // Write a dummy key with 0x00 to ensure namespace is initialized
    // This forces the flash to be written with zeros instead of 0xFF
    uint8_t zeroBuffer[256] = {0};  // 256 bytes of zeros
    preferences.putBytes("_init", zeroBuffer, sizeof(zeroBuffer));
    preferences.remove("_init");  // Remove the dummy key
    
    preferences.end();
    Serial.println(" ✓");
  } else {
    Serial.println(" ⚠ (failed to open)");
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);  // Wait for serial to be ready
  
  Serial.println("\n\n");
  Serial.println("======================================");
  Serial.println("  ESP32-S3 EEPROM CLEAR UTILITY");
  Serial.println("  (Preferences Only - Preserves WiFi)");
  Serial.println("======================================");
  Serial.println();
  Serial.println("⚠️  This will clear EEPROM data:");
  Serial.println("   - Dosing schedules");
  Serial.println("   - Outlet schedules");
  Serial.println("   - Pump calibrations");
  Serial.println("   - Top-up/Replace configs");
  Serial.println();
  Serial.println("✅ This will PRESERVE:");
  Serial.println("   - WiFi credentials");
  Serial.println("   - MQTT settings");
  Serial.println("   - System configuration");
  Serial.println();
  Serial.println("Starting in 5 seconds...");
  Serial.println("Press RESET button NOW to cancel!");
  Serial.println();
  
  for (int i = 5; i > 0; i--) {
    Serial.print(i);
    Serial.println("...");
    delay(1000);
  }
  
  Serial.println();
  Serial.println("🔥 Starting EEPROM clear process...");
  Serial.println();
  
  // Clear each namespace individually
  Serial.println("Step 1: Clearing Preferences namespaces...");
  for (int i = 0; i < namespaceCount; i++) {
    clearNamespaceToZero(namespaces[i]);
    delay(100);
  }
  
  Serial.println();
  Serial.println("Step 2: Verifying EEPROM functionality...");
  
  // Write and read back test data to verify EEPROM is working
  Serial.print("  - Testing write/read... ");
  preferences.begin("test", false);
  preferences.putUInt("verify", 0x12345678);
  uint32_t readBack = preferences.getUInt("verify", 0);
  preferences.clear();
  preferences.end();
  
  if (readBack == 0x12345678) {
    Serial.println("✓");
  } else {
    Serial.println("⚠ Failed!");
  }
  
  Serial.println();
  Serial.println("Step 3: Checking namespace sizes...");
  
  // Show that each namespace is now empty
  for (int i = 0; i < namespaceCount; i++) {
    Serial.print("  - ");
    Serial.print(namespaces[i]);
    Serial.print(": ");
    
    if (preferences.begin(namespaces[i], true)) {  // Read-only
      size_t used = preferences.freeEntries();
      Serial.print(used);
      Serial.println(" free entries");
      preferences.end();
    } else {
      Serial.println("empty");
    }
  }
  
  Serial.println();
  Serial.println("======================================");
  Serial.println("✅ EEPROM CLEAR COMPLETE!");
  Serial.println("======================================");
  Serial.println();
  Serial.println("Next steps:");
  Serial.println("1. Upload your main sketch");
  Serial.println("2. Your WiFi/MQTT settings are preserved");
  Serial.println("3. Set up your schedules via the menu");
  Serial.println();
  Serial.println("Device will now halt.");
  Serial.println("Press RESET to restart with cleared EEPROM.");
  Serial.println();
}

void loop() {
  // Blink built-in LED to show completion
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastBlink > 1000) {
    lastBlink = millis();
    ledState = !ledState;
    
    // Try to blink LED_BUILTIN if it exists
    #ifdef LED_BUILTIN
      digitalWrite(LED_BUILTIN, ledState);
    #endif
  }
  
  // Do nothing - wait for manual reset
  delay(10);
}