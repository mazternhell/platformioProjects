/*
 * Hardware.cpp
 * 
 * Implementation of hardware I/O functions.
 */

#include "Hardware.h"
#include <ArduinoJson.h>

// ==================================================
// HARDWARE INITIALIZATION
// ==================================================
void initHardware() {
  // Heartbeat LED (built-in)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Float switches (INPUT_PULLDOWN for PC817 optocoupler)
  pinMode(FLOAT_FULL, INPUT_PULLDOWN);
  pinMode(FLOAT_LOW, INPUT_PULLDOWN);
  pinMode(FLOAT_EMPTY, INPUT_PULLDOWN);

  // Relays (OUTPUT, default OFF/LOW)
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);
  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);

  // LED Indicators (OUTPUT, default OFF/LOW)
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);

  // WS2812B RGB LED
  ws2812b.begin();
  ws2812b.setBrightness(50);
  ws2812b.clear();
  ws2812b.show();

  // Setup PWM for peristaltic pumps
  ledcSetup(0, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(3, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(PUMP_1, 0);
  ledcAttachPin(PUMP_2, 1);
  ledcAttachPin(PUMP_3, 2);
  ledcAttachPin(PUMP_4, 3);

  // Set all pumps to 0%
  ledcWrite(0, 0);
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);

  // NOTE: Rotary encoder setup moved to main.cpp (must be with ISR)

  // UART for expansion
  Serial2.begin(9600, SERIAL_8N1, UART_RX, UART_TX);
}

// ==================================================
// ENCODER UPDATE (Polling)
// ==================================================
void updateEncoder() {
  // Copy volatile encoderPosition to hardware struct atomically
  noInterrupts();
  hardware.encoderPosition = encoderPosition;
  interrupts();

  // Button polling with debouncing
  currentButtonState = digitalRead(ENCODER_SW);
  unsigned long currentTime = millis();

  // Detect button press (HIGH→LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (currentTime - lastButtonTime > BUTTON_DEBOUNCE) {
      buttonPressTime = currentTime;
      hardware.encoderButton = true;
      encoderButtonPressed = true;
      lastButtonTime = currentTime;
    }
  }
  // Detect long press
  else if (currentButtonState == LOW && encoderButtonPressed) {
    if (currentTime - buttonPressTime > BUTTON_LONG_PRESS_MS) {
      encoderButtonPressed = false;
    }
  }
  // Detect button release
  else if (lastButtonState == LOW && currentButtonState == HIGH) {
    encoderButtonPressed = false;
  }
  // Backup clear
  else if (hardware.encoderButton && currentButtonState == HIGH) {
    hardware.encoderButton = false;
  }

  lastButtonState = currentButtonState;
}

// ==================================================
// FLOAT SWITCHES UPDATE
// ==================================================
void updateFloatSwitches(unsigned long currentTime) {
  if ((unsigned long)(currentTime - lastFloatCheck) < FLOAT_CHECK_INTERVAL) return;
  lastFloatCheck = currentTime;

  int fullReading = digitalRead(FLOAT_FULL);
  int lowReading = digitalRead(FLOAT_LOW);
  int emptyReading = digitalRead(FLOAT_EMPTY);

  // Debounce FULL switch
  if (fullReading != lastFloatReading[0]) {
    lastFloatDebounce[0] = currentTime;
  }
  if ((currentTime - lastFloatDebounce[0]) > FLOAT_DEBOUNCE_DELAY) {
    if (fullReading != floatState[0]) {
      floatState[0] = fullReading;
      bool newFull = (fullReading == HIGH);
      if (newFull != hardware.floatFull) {
        hardware.floatFull = newFull;
      }
    }
  }
  lastFloatReading[0] = fullReading;

  // Debounce LOW switch
  if (lowReading != lastFloatReading[1]) {
    lastFloatDebounce[1] = currentTime;
  }
  if ((currentTime - lastFloatDebounce[1]) > FLOAT_DEBOUNCE_DELAY) {
    if (lowReading != floatState[1]) {
      floatState[1] = lowReading;
      bool newLow = (lowReading == HIGH);
      if (newLow != hardware.floatLow) {
        hardware.floatLow = newLow;
      }
    }
  }
  lastFloatReading[1] = lowReading;

  // Debounce EMPTY switch
  if (emptyReading != lastFloatReading[2]) {
    lastFloatDebounce[2] = currentTime;
  }
  if ((currentTime - lastFloatDebounce[2]) > FLOAT_DEBOUNCE_DELAY) {
    if (emptyReading != floatState[2]) {
      floatState[2] = emptyReading;
      bool newEmpty = (emptyReading == HIGH);
      if (newEmpty != hardware.floatEmpty) {
        hardware.floatEmpty = newEmpty;
      }
    }
  }
  lastFloatReading[2] = emptyReading;
}

// ==================================================
// TOUCH SENSORS UPDATE
// ==================================================
void updateTouchSensors(unsigned long currentTime) {
  if ((unsigned long)(currentTime - lastTouchCheck) < TOUCH_CHECK_INTERVAL) return;
  lastTouchCheck = currentTime;

  uint16_t touch1Val = touchRead(TOUCH_1);
  uint16_t touch2Val = touchRead(TOUCH_2);
  uint16_t touch3Val = touchRead(TOUCH_3);
  uint16_t touch4Val = touchRead(TOUCH_4);

  bool newTouch1 = (touch1Val < TOUCH_THRESHOLD);
  bool newTouch2 = (touch2Val < TOUCH_THRESHOLD);
  bool newTouch3 = (touch3Val < TOUCH_THRESHOLD);
  bool newTouch4 = (touch4Val < TOUCH_THRESHOLD);

  if (newTouch1 != hardware.touch1) hardware.touch1 = newTouch1;
  if (newTouch2 != hardware.touch2) hardware.touch2 = newTouch2;
  if (newTouch3 != hardware.touch3) hardware.touch3 = newTouch3;
  if (newTouch4 != hardware.touch4) hardware.touch4 = newTouch4;
}

// ==================================================
// RELAY CONTROL
// ==================================================
void setRelay(uint8_t relay, bool state) {
  uint8_t pin;

  switch(relay) {
    case 1: pin = RELAY_1; hardware.relay1 = state; break;
    case 2: pin = RELAY_2; hardware.relay2 = state; break;
    case 3: pin = RELAY_3; hardware.relay3 = state; break;
    case 4: pin = RELAY_4; hardware.relay4 = state; break;
    default: return;
  }

  digitalWrite(pin, state ? HIGH : LOW);
}

// ==================================================
// PUMP CONTROL
// ==================================================
void setPumpSpeed(uint8_t pump, uint8_t speed) {
  if (speed > 100) speed = 100;

  uint8_t pwmValue = map(speed, 0, 100, 0, 255);
  uint8_t channel;

  switch(pump) {
    case 1:
      channel = 0;
      hardware.pump1Speed = speed;
      break;
    case 2:
      channel = 1;
      hardware.pump2Speed = speed;
      break;
    case 3:
      channel = 2;
      hardware.pump3Speed = speed;
      break;
    case 4:
      channel = 3;
      hardware.pump4Speed = speed;
      break;
    default: return;
  }

  ledcWrite(channel, pwmValue);
}

uint8_t getPumpSpeed(uint8_t pump) {
  switch(pump) {
    case 1: return hardware.pump1Speed;
    case 2: return hardware.pump2Speed;
    case 3: return hardware.pump3Speed;
    case 4: return hardware.pump4Speed;
    default: return 0;
  }
}

// ==================================================
// FLOAT SWITCH READING
// ==================================================
bool getFloatSwitch(uint8_t level) {
  switch(level) {
    case 0: return hardware.floatEmpty;
    case 1: return hardware.floatLow;
    case 2: return hardware.floatFull;
    default: return false;
  }
}

// ==================================================
// LED CONTROL
// ==================================================
void setLED(uint8_t led, bool state) {
  uint8_t pin;

  switch(led) {
    case 1: pin = LED_1; hardware.led1 = state; break;
    case 2: pin = LED_2; hardware.led2 = state; break;
    case 3: pin = LED_3; hardware.led3 = state; break;
    case 4: pin = LED_4; hardware.led4 = state; break;
    default: return;
  }

  digitalWrite(pin, state ? HIGH : LOW);
}

// ==================================================
// WS2812B RGB LED
// ==================================================
void setWS2812B(uint8_t r, uint8_t g, uint8_t b) {
  hardware.ws2812b_r = r;
  hardware.ws2812b_g = g;
  hardware.ws2812b_b = b;

  ws2812b.setPixelColor(0, ws2812b.Color(r, g, b));
  ws2812b.show();
}

// ==================================================
// JSON EXPORT
// ==================================================
String getHardwareJSON() {
  StaticJsonDocument<768> doc;

  doc["floatFull"] = hardware.floatFull;
  doc["floatLow"] = hardware.floatLow;
  doc["floatEmpty"] = hardware.floatEmpty;

  doc["relay1"] = hardware.relay1;
  doc["relay2"] = hardware.relay2;
  doc["relay3"] = hardware.relay3;
  doc["relay4"] = hardware.relay4;

  doc["pump1"] = hardware.pump1Speed;
  doc["pump2"] = hardware.pump2Speed;
  doc["pump3"] = hardware.pump3Speed;
  doc["pump4"] = hardware.pump4Speed;

  doc["led1"] = hardware.led1;
  doc["led2"] = hardware.led2;
  doc["led3"] = hardware.led3;
  doc["led4"] = hardware.led4;

  doc["ws2812b_r"] = hardware.ws2812b_r;
  doc["ws2812b_g"] = hardware.ws2812b_g;
  doc["ws2812b_b"] = hardware.ws2812b_b;

  doc["encoderPos"] = hardware.encoderPosition;
  doc["encoderBtn"] = hardware.encoderButton;

  doc["touch1"] = hardware.touch1;
  doc["touch2"] = hardware.touch2;
  doc["touch3"] = hardware.touch3;
  doc["touch4"] = hardware.touch4;

  String output;
  serializeJson(doc, output);
  return output;
}
