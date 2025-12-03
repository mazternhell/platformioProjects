#include <Arduino.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>

// Your credentials
const char* ssid = "MERCUSYS_5085_IOT";
const char* password = "vUY9HN33";

// HiveMQ Cloud credentials
const char* mqtt_server = "e53e8b3385094339a4a8e5e112303d32.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // TLS port
const char* mqtt_user = "hydroUnit1";
const char* mqtt_pass = "T@chym3t3r";

AsyncMqttClient mqttClient;

// LED for testing
#define LED_PIN 41  // LED 1

void onMqttConnect(bool sessionPresent) {
  Serial.println("✓ MQTT Connected!");
  digitalWrite(LED_PIN, HIGH);  // Turn on LED when connected
  
  // Subscribe to test topic
  mqttClient.subscribe("test/topic", 0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("✗ MQTT Disconnected");
  digitalWrite(LED_PIN, LOW);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.print("Message received on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("\n=== AsyncMqttClient TLS Test ===");
  
  // Setup MQTT callbacks
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  
  // Configure MQTT with TLS
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCredentials(mqtt_user, mqtt_pass);
  mqttClient.setSecure(true);  // Enable TLS
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✓ WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Connect to MQTT
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void loop() {
  // Nothing needed here - AsyncMqttClient handles everything
  delay(100);
}