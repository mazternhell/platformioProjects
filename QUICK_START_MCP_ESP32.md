# Quick Start Guide: MCP + ESP32 Development

**Copy-paste this at the start of any new Claude conversation for ESP32 work.**

---

## 🚀 Quick Setup

**Step 1: Load My Development Skill**
```
Read my ESP32 development skill at:
/Users/mazternhell/Documents/PlatformIO/Projects/esp32-platformio-skill/SKILL.md
```

**Step 2: Tell Claude the Project**
```
I'm working on [project name] in my PlatformIO Projects folder.
```

---

## 📋 My 8 Core Rules (MANDATORY)

1. ⏱️ **Non-blocking delays** - Use `millis()`, never `delay()`
2. 🚩 **Timer interrupts + flags** - Hardware timers for precision
3. 🔄 **Dual-core from start** - Core 0: Network, Core 1: App
4. 📦 **Modularize** - Max 500 lines per file
5. 💾 **Optimize memory** - Flash/SPIFFS/PSRAM
6. ⚡ **Keep optimization in mind** - Always
7. 📝 **Minimal docs** - Only when requested
8. 🧪 **Test incrementally** - Hardware first, then integrate

---

## 🎯 File Reading Protocol

**ALWAYS follow this order:**

1. Read `platformio.ini` first (board, libraries, settings)
2. Read `src/config.h` second (pins, constants)
3. Preview `main.cpp` (head: 100 lines)
4. Read module `.h` files (interface)
5. Read module `.cpp` files ONLY if needed

**Token Optimization:**
- Use `head` parameter for large files
- Read ONLY relevant modules for the task
- Don't read entire project at once

---

## 📁 My Projects

**Location:** `/Users/mazternhell/Documents/PlatformIO/Projects/`

**GitHub:** nelsondolot

**Repositories:**
- `hydroponics-controller-v2` (ESP32-S3, MQTT, sensors)
- `esp32-s3-devkitc-project` (ESP32-S3 dev)
- `esp32-platformio-skill` (reference guide)

---

## 🛠️ MCP Tools Available

**Filesystem:** Read/write local files  
**Git:** Status, history (limited on Windows)  
**GitHub:** Read repos, create issues, PRs

**Limitations:**
- Bash commands don't work on Windows
- Initial git pushes must be manual
- Provide me with exact commands to run

---

## 📐 Standard Module Structure

```
project/
├── platformio.ini
├── src/
│   ├── main.cpp (100-200 lines)
│   ├── config.h
│   ├── wifi_manager.cpp/h
│   ├── sensor_handler.cpp/h
│   └── mqtt_handler.cpp/h
├── include/
├── lib/
└── test/
```

---

## 🔄 Dual-Core Distribution

**Core 0 (Network):** WiFi, Bluetooth, MQTT, HTTP  
**Core 1 (Application):** Sensors, Display, UI, Logic

```cpp
xTaskCreatePinnedToCore(taskFunc, "Name", 4096, NULL, 1, NULL, 0); // Core 0
xTaskCreatePinnedToCore(taskFunc, "Name", 4096, NULL, 1, NULL, 1); // Core 1
```

---

## ⚡ Non-Blocking Pattern

```cpp
unsigned long previousMillis = 0;
const long interval = 1000;

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        // Do work
    }
}
```

---

## 🚩 Timer Interrupt Pattern

```cpp
volatile bool timerFlag = false;
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
    timerFlag = true;
}

void setup() {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);
}

void loop() {
    if (timerFlag) {
        timerFlag = false;
        // Handle event
    }
}
```

---

## 💡 Example Requests

**Analysis:**
```
"Analyze my hydroponics controller following my ESP32 rules"
```

**New Module:**
```
"Add a temperature sensor module following my modularization patterns"
```

**Refactoring:**
```
"Modularize my main.cpp - it's too long and violates my 500-line rule"
```

**Debugging:**
```
"My WiFi keeps disconnecting. Fix it following my non-blocking pattern"
```

**GitHub:**
```
"Create an issue in hydroponics-controller-v2 to track OTA updates"
```

---

## ⚠️ Critical Reminders

- **Read the skill FIRST** before any work
- **Follow all 8 rules** without exception
- **Read platformio.ini FIRST** always
- **Modularize aggressively** - 500 line max
- **Document minimally** unless asked
- **Test hardware incrementally**

---

## 🔗 Full Documentation

For complete details, see: `MCP_ESP32_DEVELOPMENT_GUIDE.md`

---

**Location:** Quezon City, Metro Manila, PH  
**Last Updated:** November 6, 2025