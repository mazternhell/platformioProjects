# Comprehensive Guide: Using MCP Tools for ESP32 PlatformIO Development

**Purpose:** This document provides complete instructions for Claude instances to effectively use Model Context Protocol (MCP) tools when developing ESP32 projects with PlatformIO and Arduino framework.

**Author:** Nelson (nelsondolot)  
**Last Updated:** November 6, 2025  
**Version:** 1.0

---

## Table of Contents

1. [MCP Setup Overview](#mcp-setup-overview)
2. [Available MCP Tools](#available-mcp-tools)
3. [Core Development Rules](#core-development-rules)
4. [Optimal Workflow](#optimal-workflow)
5. [Token Optimization Strategy](#token-optimization-strategy)
6. [File Reading Protocol](#file-reading-protocol)
7. [GitHub Integration Workflow](#github-integration-workflow)
8. [Common Commands & Patterns](#common-commands--patterns)
9. [Troubleshooting](#troubleshooting)
10. [Example Conversation Starters](#example-conversation-starters)

---

## MCP Setup Overview

### What is MCP?

Model Context Protocol (MCP) is a standardized way for Claude to access external data sources and tools. For ESP32 development, MCP enables Claude to:

- Access local project files directly
- Read and analyze code structure
- Interact with GitHub repositories
- Follow user-specific development conventions

### Current MCP Configuration

The user has the following MCP servers configured:

```json
{
  "mcpServers": {
    "filesystem": {
      "command": "npx",
      "args": [
        "-y",
        "@modelcontextprotocol/server-filesystem",
        "C:\\Users\\mazternhell\\Desktop",
        "C:\\Users\\mazternhell\\Downloads",
        "C:\\Users\\mazternhell\\Documents\\PlatformIO\\Projects"
      ]
    },
    "git": {
      "command": "npx",
      "args": [
        "-y",
        "@modelcontextprotocol/server-git",
        "--repository",
        "C:\\Users\\mazternhell\\Documents\\PlatformIO\\Projects"
      ]
    },
    "github": {
      "command": "npx",
      "args": [
        "-y",
        "@modelcontextprotocol/server-github"
      ],
      "env": {
        "GITHUB_PERSONAL_ACCESS_TOKEN": "[configured]"
      }
    }
  }
}
```

**Note:** File paths use Windows format with double backslashes.

---

## Available MCP Tools

### 1. Filesystem MCP

**Access Path:** `C:\Users\mazternhell\Documents\PlatformIO\Projects`  
**GitHub Username:** nelsondolot

**What you can do:**
- ✅ Read any file in the Projects directory
- ✅ List directory contents
- ✅ Get file information (size, modified date)
- ✅ Create new files
- ✅ Edit existing files
- ✅ Search for files by name/pattern

**Important:** Use Unix-style paths in MCP commands: `/Users/mazternhell/Documents/PlatformIO/Projects/`

**Key Functions:**
- `filesystem:read_text_file` - Read complete file contents
- `filesystem:list_directory` - List files and folders
- `filesystem:directory_tree` - Get recursive structure
- `filesystem:write_file` - Create new files
- `filesystem:edit_file` - Modify existing files
- `filesystem:search_files` - Find files by pattern

### 2. Git MCP

**What you can do:**
- ✅ Check repository status
- ✅ View commit history
- ✅ See file diffs
- ✅ Manage branches

**Limitations:**
- ❌ Bash commands don't work reliably on Windows
- ❌ User must run git commands manually when needed

**Workaround:** When git operations are needed, provide exact commands for the user to run.

### 3. GitHub MCP

**GitHub Username:** nelsondolot  
**Active Repositories:**
- `hydroponics-controller-v2` (private)
- `esp32-s3-devkitc-project` (private)
- `esp32-platformio-skill` (private)

**What you can do:**
- ✅ Read files from repositories
- ✅ Create issues
- ✅ Comment on issues
- ✅ Create pull requests
- ✅ Search repositories
- ✅ List commits
- ✅ Create branches
- ✅ Update files (after initial push)

**Key Functions:**
- `github:get_file_contents` - Read files from GitHub
- `github:create_issue` - Track bugs/features
- `github:create_pull_request` - Propose changes
- `github:search_repositories` - Find repos
- `github:list_commits` - View commit history

**Important:** Initial pushes to empty repos must be done manually by user. After that, file updates work fine.

---

## Core Development Rules

**CRITICAL:** These are mandatory principles that MUST be followed in ALL ESP32 projects:

### 1. Use Non-Blocking Delays ⏱️
- ❌ NEVER use `delay()` in production code
- ✅ ALWAYS use `millis()` for timing
- ✅ Implement interval timers for periodic tasks

**Example:**
```cpp
unsigned long previousMillis = 0;
const long interval = 1000;

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        // Execute timed action
    }
}
```

### 2. Use Timer Interrupts with Flagging 🚩
- ✅ Leverage ESP32 hardware timers
- ✅ Use volatile flags for ISR communication
- ✅ Keep ISR functions minimal

**Example:**
```cpp
volatile bool timerFlag = false;
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
    timerFlag = true;
}

void setup() {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000, true);  // 1 second
    timerAlarmEnable(timer);
}

void loop() {
    if (timerFlag) {
        timerFlag = false;
        // Handle timer event
    }
}
```

### 3. Leverage Dual-Core Architecture 🔄
- ✅ Use BOTH cores from the start
- ✅ Core 0: WiFi/Network/MQTT
- ✅ Core 1: Sensors/Display/Application logic
- ✅ Use `xTaskCreatePinnedToCore()`

**Example:**
```cpp
xTaskCreatePinnedToCore(
    networkTask,      // Function
    "NetworkTask",    // Name
    4096,            // Stack size
    NULL,            // Parameters
    1,               // Priority
    NULL,            // Task handle
    0                // Core 0 (network)
);

xTaskCreatePinnedToCore(
    sensorTask,
    "SensorTask",
    4096,
    NULL,
    1,
    NULL,
    1                // Core 1 (application)
);
```

### 4. Modularize Code 📦
- ✅ Split functionality into logical modules
- ✅ Maximum 500 lines per file
- ✅ Separate .h and .cpp files
- ✅ Single responsibility per module

**Standard Structure:**
```
project/
├── platformio.ini
├── src/
│   ├── main.cpp              (100-200 lines max)
│   ├── config.h              (pin definitions, constants)
│   ├── wifi_manager.cpp/h
│   ├── sensor_handler.cpp/h
│   ├── pump_controller.cpp/h
│   └── mqtt_handler.cpp/h
├── include/
├── lib/
└── test/
```

### 5. Optimize Memory Usage 💾
- ✅ Monitor heap: `ESP.getFreeHeap()`
- ✅ Use Flash for constants: `const char[] PROGMEM`
- ✅ Use SPIFFS/LittleFS for file storage
- ✅ Use PSRAM for large buffers (if available)
- ✅ Pre-allocate buffers when possible

### 6. Keep Optimization in Mind ⚡
- ✅ Avoid floating-point when possible
- ✅ Minimize String usage (prefer char arrays)
- ✅ Use appropriate data types (uint8_t vs int)
- ✅ Cache frequently accessed values
- ✅ Optimize loop() execution time

### 7. Minimal Documentation 📝
- ✅ Document ONLY when explicitly requested
- ✅ Focus on code quality over verbose comments
- ✅ Let code structure speak for itself
- ❌ Don't create comprehensive docs by default

### 8. Test Hardware Incrementally 🧪
- ✅ Test individual sensors first
- ✅ Verify each component separately
- ✅ Use separate test sketches
- ✅ Monitor serial output extensively
- ✅ Integrate one module at a time

---

## Optimal Workflow

### Starting a New Conversation

**Step 1: Load the ESP32 Development Skill**

ALWAYS begin by reading the user's custom skill:

```
Read my ESP32 development skill at:
/Users/mazternhell/Documents/PlatformIO/Projects/esp32-platformio-skill/SKILL.md
```

This skill contains:
- The 8 core development rules (above)
- Token optimization strategies
- Modularization patterns
- Common code patterns
- Module templates

**Step 2: Identify the Project**

Ask which project to work on or read the project list:

```
filesystem:list_directory
path: /Users/mazternhell/Documents/PlatformIO/Projects
```

Current projects:
- `hydroponicsControllerV2` - ESP32-S3 based hydroponics controller
- `251027-220208-esp32-s3-devkitc-1` - ESP32-S3 development project
- `esp32-platformio-skill` - Custom MCP skill (reference material)

**Step 3: Follow the File Reading Protocol**

See [File Reading Protocol](#file-reading-protocol) section below.

---

## Token Optimization Strategy

### The Golden Rule
**Read ONLY what's needed, WHEN it's needed.**

### Priority Reading Order

1. **platformio.ini** (ALWAYS read first)
   - ~100 tokens
   - Critical context about board, libraries, settings

2. **src/config.h** (Read second)
   - Pin definitions
   - Constants
   - Configuration settings

3. **src/main.cpp** (Read for overview)
   - Entry point
   - Task creation
   - Overall structure
   - Use `head` parameter to preview first 50-100 lines

4. **Module .h files** (Before .cpp files)
   - Interface overview
   - Function signatures
   - Public API

5. **Module .cpp files** (Only when needed)
   - Implementation details
   - Only read when debugging or understanding specific logic

### What NOT to Read

- ❌ Compiled code in `.pio/` directory
- ❌ Arduino/ESP32 library internals (unless debugging library issues)
- ❌ Entire project at once
- ❌ Verbose documentation files (unless requested)

### Using `head` Parameter

For large files, preview first:

```
filesystem:read_text_file
path: /Users/mazternhell/Documents/PlatformIO/Projects/project/src/main.cpp
head: 100  // First 100 lines only
```

### Identify Relevant Modules

Based on user's task, read ONLY relevant modules:
- WiFi task? → Read `wifi_manager.cpp/h`
- Sensor issue? → Read `sensor_handler.cpp/h`
- MQTT problem? → Read `mqtt_handler.cpp/h`

**Don't read everything!**

---

## File Reading Protocol

### Step-by-Step Process

**For ANY ESP32 project task, follow this exact sequence:**

#### 1. Read platformio.ini

```
filesystem:read_text_file
path: /Users/mazternhell/Documents/PlatformIO/Projects/PROJECT_NAME/platformio.ini
```

**Extract:**
- Board type (esp32dev, esp32-s3-devkitc-1, etc.)
- Framework version
- Library dependencies
- Build flags
- Upload settings

#### 2. Check Project Structure

```
filesystem:list_directory
path: /Users/mazternhell/Documents/PlatformIO/Projects/PROJECT_NAME/src
```

**Identify:**
- main.cpp location
- Module organization
- Configuration files

#### 3. Read Config (if exists)

```
filesystem:read_text_file
path: /Users/mazternhell/Documents/PlatformIO/Projects/PROJECT_NAME/src/config.h
```

#### 4. Preview main.cpp

```
filesystem:read_text_file
path: /Users/mazternhell/Documents/PlatformIO/Projects/PROJECT_NAME/src/main.cpp
head: 100  // Preview only
```

**Analyze:**
- Includes (what modules are used)
- Task creation (dual-core usage)
- setup() and loop() structure

#### 5. Read Relevant Modules Only

Based on the task, read specific modules:

```
// Read header first (interface)
filesystem:read_text_file
path: /Users/mazternhell/Documents/PlatformIO/Projects/PROJECT_NAME/src/MODULE_NAME.h

// Then implementation if needed
filesystem:read_text_file
path: /Users/mazternhell/Documents/PlatformIO/Projects/PROJECT_NAME/src/MODULE_NAME.cpp
```

### Example: "Fix WiFi connection issue"

**Efficient approach:**
1. Read `platformio.ini` → Check WiFi library version
2. Read `src/config.h` → Check WiFi credentials setup
3. Read `wifi_manager.h` → Understand interface
4. Read `wifi_manager.cpp` → Analyze implementation
5. Read `main.cpp` (head: 50) → See how WiFi is initialized

**Total tokens:** ~2000-3000

**Inefficient approach (DON'T DO THIS):**
1. Read entire project with `directory_tree`
2. Read all .cpp files
3. Read library source code

**Total tokens:** 20,000+ (wasted!)

---

## GitHub Integration Workflow

### Reading Code from GitHub

**Why use GitHub MCP?**
- Verify what's currently in the repo
- Review pushed changes
- Check file history
- Collaborate via issues/PRs

**How to read files:**

```
github:get_file_contents
owner: nelsondolot
repo: hydroponics-controller-v2
path: platformio.ini
branch: main  // optional
```

### Creating Issues

**When to create issues:**
- User finds a bug
- Feature request identified
- TODO items to track

**How to create:**

```
github:create_issue
owner: nelsondolot
repo: hydroponics-controller-v2
title: "Add OTA update functionality"
body: "Implement Over-The-Air updates for remote firmware deployment..."
labels: ["enhancement", "feature"]  // optional
```

### Creating Pull Requests

**Workflow:**
1. User makes changes locally and pushes
2. Create PR for review

```
github:create_pull_request
owner: nelsondolot
repo: hydroponics-controller-v2
title: "Add temperature sensor module"
head: "feature/temperature-sensor"
base: "main"
body: "This PR adds a new modularized temperature sensor handler..."
```

### Limitations

**What Works:**
- ✅ Reading files from GitHub
- ✅ Creating issues
- ✅ Commenting on issues
- ✅ Creating PRs
- ✅ Searching code

**What Doesn't Work:**
- ❌ Initial push to empty repos (user must do manually)
- ❌ Direct commits to repos (use local filesystem instead)

**Workaround:** Use filesystem MCP to modify local files, then provide user with git commands to commit and push.

---

## Common Commands & Patterns

### Analyzing a Project

```markdown
"Analyze my hydroponics controller project"

STEPS:
1. Read platformio.ini
2. List src/ directory
3. Read main.cpp (head: 100)
4. Identify modules
5. Report: board type, libraries, structure, potential issues
```

### Adding a New Module

```markdown
"Add a temperature sensor module to my project"

STEPS:
1. Read platformio.ini (check existing sensors)
2. Read src/config.h (check available pins)
3. Create sensor_temp.h (interface)
4. Create sensor_temp.cpp (implementation)
5. Update main.cpp (integrate module)
6. Follow modularization patterns from skill
```

### Refactoring for Modularization

```markdown
"Modularize my main.cpp file"

STEPS:
1. Read main.cpp (full file)
2. Identify logical sections (WiFi, sensors, MQTT, display)
3. Create separate modules for each
4. Follow standard module template
5. Update main.cpp to use modules
6. Ensure dual-core task distribution
```

### Debugging an Issue

```markdown
"My MQTT connection keeps dropping"

STEPS:
1. Read platformio.ini (check MQTT library version)
2. Read mqtt_handler.cpp (analyze reconnection logic)
3. Read main.cpp (check task priority and core assignment)
4. Check for blocking code (delay() usage)
5. Suggest fixes following non-blocking patterns
```

### Creating Documentation (Only When Requested)

```markdown
"Document my sensor module"

STEPS:
1. Read sensor_handler.h/cpp
2. Create comprehensive comments
3. Generate README with usage examples
4. Note: Only do this when explicitly requested!
```

---

## Troubleshooting

### Issue: Can't Access Files

**Symptom:** `Access denied - path outside allowed directories`

**Solution:**
- Check path format: Use `/Users/mazternhell/...` (Unix-style)
- Verify path is within: `C:\Users\mazternhell\Documents\PlatformIO\Projects`
- Don't access files outside allowed directories

### Issue: GitHub Operations Failing

**Symptom:** `Tool execution failed` on GitHub operations

**Possible Causes:**
1. Token permissions insufficient
2. Repository doesn't exist
3. Network issues
4. Trying to push to empty repo

**Solution:**
- Verify repo exists: `github:search_repositories`
- Check repo name and owner (nelsondolot)
- For empty repos, user must do initial push manually

### Issue: Git Commands Not Working

**Symptom:** Bash commands fail with path errors

**Explanation:** Windows path compatibility issues with bash_tool

**Solution:**
- Don't try to run git commands via MCP
- Provide exact commands for user to run in their terminal
- Use GitHub MCP for reading repo state

### Issue: Too Many Tokens Used

**Symptom:** Running out of context window space

**Solution:**
- Re-read the [Token Optimization Strategy](#token-optimization-strategy)
- Use `head` parameter more aggressively
- Read only relevant modules
- Don't read entire project at once

---

## Example Conversation Starters

### For User to Say

**Starting Fresh:**
```
"Read my ESP32 development skill and then analyze my hydroponics controller project"
```

**Specific Task:**
```
"Following my ESP32 rules, add a pH sensor module to my hydroponics controller"
```

**Debugging:**
```
"My ESP32 keeps crashing when WiFi disconnects. Use my development patterns to fix it."
```

**Refactoring:**
```
"Modularize my main.cpp following the patterns in my ESP32 skill"
```

**GitHub Integration:**
```
"Create an issue in my hydroponics-controller-v2 repo to track the OTA update feature"
```

### For Claude to Say

**Starting Response:**
```
I'll start by reading your ESP32 development skill to understand your coding standards...
[reads /Users/mazternhell/Documents/PlatformIO/Projects/esp32-platformio-skill/SKILL.md]

Now let me analyze your platformio.ini to understand the project configuration...
[reads platformio.ini]

Based on your skill, I can see you require:
1. Non-blocking delays (millis-based)
2. Dual-core task distribution
3. Modularized code structure
...
```

**When Modularizing:**
```
I'll create this module following your patterns:
1. Separate .h and .cpp files
2. Clear interface in header
3. Non-blocking update() method
4. Fits within 500 lines
5. Single responsibility

Let me create wifi_manager.h first...
[creates file]
```

**When Debugging:**
```
I've analyzed your code following the token optimization strategy:
1. Read platformio.ini ✓
2. Identified relevant module (mqtt_handler)
3. Found the issue: blocking delay() on line 45

This violates your Rule #1: Use non-blocking delays.

Here's the fix following your non-blocking pattern...
```

---

## Quick Reference Card

### MCP Paths
- **Local Projects:** `/Users/mazternhell/Documents/PlatformIO/Projects/`
- **GitHub User:** `nelsondolot`
- **Skill Location:** `esp32-platformio-skill/SKILL.md`

### Reading Order
1. **platformio.ini** (always first)
2. **config.h** (pins & constants)
3. **main.cpp** (head: 100 for overview)
4. **module.h** (interface)
5. **module.cpp** (only if needed)

### Core Rules (Always Apply)
1. ⏱️ Non-blocking delays (millis)
2. 🚩 Timer interrupts + flags
3. 🔄 Dual-core from start
4. 📦 Modularize (500 lines max)
5. 💾 Optimize memory
6. ⚡ Keep optimization in mind
7. 📝 Minimal docs
8. 🧪 Test incrementally

### GitHub Repos
- `hydroponics-controller-v2` (ESP32-S3, MQTT, sensors)
- `esp32-s3-devkitc-project` (ESP32-S3 dev)
- `esp32-platformio-skill` (development guide)

### File Limits
- main.cpp: 100-200 lines
- Module .h: 50-150 lines
- Module .cpp: 100-500 lines
- Split when > 500 lines

### Dual-Core Distribution
- **Core 0:** WiFi, Bluetooth, MQTT, HTTP
- **Core 1:** Sensors, Display, UI, Logic

---

## Important Notes

### For Claude Instances

1. **ALWAYS read the skill first** before working on any ESP32 project
2. **Follow the 8 core rules** without exception
3. **Optimize for tokens** - read only what's needed
4. **Use filesystem MCP** for local file operations
5. **Use GitHub MCP** for repository operations
6. **Don't try to run bash commands** - they won't work on Windows
7. **Provide exact commands** when user needs to run something manually
8. **Modularize aggressively** - keep files under 500 lines
9. **Document minimally** unless explicitly requested
10. **Test hardware incrementally** - never assume hardware works

### For Users

1. **Start conversations** by asking Claude to read the ESP32 skill
2. **Be specific** about which project you're working on
3. **Manually run git commands** when Claude provides them
4. **Initial repo pushes** must be done manually (after that, GitHub MCP works)
5. **Reference the skill** when asking for help
6. **Trust the process** - the token optimization really works

---

## Version History

**v1.0 (November 6, 2025)**
- Initial comprehensive guide
- Documented all MCP tools and workflows
- Added token optimization strategies
- Included troubleshooting section
- Created example conversation starters

---

## Additional Resources

### On GitHub
- Skill Repository: `nelsondolot/esp32-platformio-skill`
- Hydroponics Project: `nelsondolot/hydroponics-controller-v2`
- ESP32-S3 Project: `nelsondolot/esp32-s3-devkitc-project`

### In Projects Folder
- **SKILL.md** - Complete development skill
- **references/modularization-patterns.md** - Detailed examples
- **references/common-patterns.md** - Reusable code
- **assets/templates/** - Module templates

### MCP Documentation
- Official MCP: https://modelcontextprotocol.io/
- Anthropic Docs: https://docs.claude.com/

---

## Contact & Updates

**User:** Nelson (nelsondolot)  
**Location:** Quezon City, Metro Manila, PH  
**Focus:** ESP32/PlatformIO/Arduino development with hydroponics and IoT projects

**Note:** This guide should be updated as new MCP tools are added or workflows evolve.

---

**END OF GUIDE**

**Usage:** Copy this entire document and paste it at the beginning of a new Claude conversation when working on ESP32 projects. Claude will use it as a comprehensive reference for optimal development assistance.