#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include "DHT.h"

#define DHTPIN    13
#define DHTTYPE   DHT11
#define LED_PIN   4     // Flash LED 
#define FW_VERSION "1.0.2" // OTA Updated Version

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_server = "YOUR_HIVEMQ_CLUSTER_URL";
const int   mqtt_port   = 8883;
const char* mqtt_user   = "YOUR_MQTT_USERNAME";
const char* mqtt_pass   = "YOUR_MQTT_PASSWORD";

const char* mqtt_data_topic   = "iot/esp32cam/env";
const char* mqtt_status_topic = "iot/esp32cam/status";

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

bool dataSent      = false;
bool otaInProgress = false;
unsigned long otaWindowStart;

#define OTA_WINDOW_MS   60000      // 1 minute OTA window
#define SLEEP_MINUTES  1           // Deep sleep time

void ledOn()  { digitalWrite(LED_PIN, HIGH); }
void ledOff() { digitalWrite(LED_PIN, LOW); }

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 15000) {
      Serial.println("\nWiFi timeout");
      return;
    }
  }

  Serial.println("\nWiFi connected");
}

void connectMQTT() {
  if (mqttClient.connected()) return;

  Serial.print("Connecting to MQTT...");
  if (mqttClient.connect(
        "ESP32_CAM_ENV",
        mqtt_user,
        mqtt_pass,
        mqtt_status_topic,
        1,
        true,
        "OFFLINE")) {

    Serial.println("connected");
    mqttClient.publish(mqtt_status_topic, "ONLINE", true);
  } else {
    Serial.println("failed");
  }
}

bool publishWithRetry(const char* topic, const char* payload, int retries = 3) {
  for (int i = 1; i <= retries; i++) {
    Serial.printf("Publish attempt %d\n", i);

    if (mqttClient.publish(topic, payload, true)) {
      Serial.println("Publish success");
      return true;
    }

    Serial.println("Publish failed, retrying...");
    delay(1000);
  }

  Serial.println("Publish failed after retries");
  return false;
}

void setupOTA() {
  ArduinoOTA.setHostname("ESP32_CAM_ENV");

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    Serial.println("OTA started (sleep blocked)");
  });

  ArduinoOTA.onEnd([]() {
    otaInProgress = false;
    Serial.println("OTA finished");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
    Serial.printf("OTA error: %u\n", error);
  });

  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);
  pinMode(LED_PIN, OUTPUT);
  ledOff();

  WiFi.mode(WIFI_STA);
  connectWiFi();

  secureClient.setInsecure();      
  mqttClient.setServer(mqtt_server, mqtt_port);

  dht.begin();
  delay(2000);                     

  setupOTA();
  otaWindowStart = millis();
}


void loop() {
  ArduinoOTA.handle();

  connectWiFi();
  connectMQTT();
  mqttClient.loop();

  if (!dataSent && mqttClient.connected()) {
    ledOn();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
      char payload[128];
      snprintf(payload, sizeof(payload),
        "{\"device\":\"ESP32_CAM_ENV\",\"temp\":%.2f,\"hum\":%.2f}",
        temp, hum);

      if (publishWithRetry(mqtt_data_topic, payload)) {
        dataSent = true;
      }
    }
  }

  if (!otaInProgress && millis() - otaWindowStart > OTA_WINDOW_MS) {
    Serial.println("Going to deep sleep");

    mqttClient.publish(mqtt_status_topic, "SLEEPING", true);
    mqttClient.disconnect();

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    ledOff();

    esp_sleep_enable_timer_wakeup(SLEEP_MINUTES * 60ULL * 1000000ULL);
    esp_deep_sleep_start();
  }
}
