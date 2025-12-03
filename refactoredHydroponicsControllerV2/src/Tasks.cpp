/*
 * Tasks.cpp
 * 
 * Implementation of FreeRTOS tasks for dual-core operation.
 */

#include "Tasks.h"
#include <esp_task_wdt.h>

// Forward declarations for menu functions (implemented in MenuSystem)
extern bool checkMenuTimeout(unsigned long currentTime);
extern void handleMenuNavigation();

// ==================================================
// DISPLAY TASK - Core 1
// ==================================================
// Handles menu rendering and display updates at 20 FPS
void DisplayTask(void *parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t frequency = pdMS_TO_TICKS(50); // 50ms = 20 FPS

  for(;;) {
    // Feed watchdog at start of loop
    esp_task_wdt_reset();
    
    // Check menu timeout FIRST before handling navigation
    checkMenuTimeout(millis());

    // Handle menu navigation
    handleMenuNavigation();

    // Draw menu
    drawMenu();

    // Maintain consistent timing
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// ==================================================
// SENSOR TASK - Core 1
// ==================================================
// Handles float switches and touch sensors at 10 Hz
void SensorTask(void *parameter) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t frequency = pdMS_TO_TICKS(100); // 100ms

  for(;;) {
    // Feed watchdog at start of loop
    esp_task_wdt_reset();
    
    unsigned long currentTime = millis();

    // Update float switches
    updateFloatSwitches(currentTime);

    // Update touch sensors
    updateTouchSensors(currentTime);

    // Maintain consistent timing
    vTaskDelayUntil(&lastWakeTime, frequency);
  }
}

// ==================================================
// TASK INITIALIZATION
// ==================================================
void startTasks() {
  // Feed watchdog before creating tasks
  esp_task_wdt_reset();

  // Display task - handles menu rendering (Core 1, high priority)
  xTaskCreatePinnedToCore(
    DisplayTask,
    "DisplayTask",
    4096,
    NULL,
    2,
    &DisplayTaskHandle,
    CORE_1
  );

  // Sensor task - handles float switches and touch (Core 1, medium priority)
  xTaskCreatePinnedToCore(
    SensorTask,
    "SensorTask",
    2048,
    NULL,
    1,
    &SensorTaskHandle,
    CORE_1
  );

  Serial.println("✓ FreeRTOS tasks started");
}
