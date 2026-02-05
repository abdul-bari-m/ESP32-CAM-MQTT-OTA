#include <WiFi.h>
#include <ArduinoOTA.h>

/* ---------- WiFi Credentials ---------- */
const char* ssid = "BARIpxl";
const char* password = "bari12344";

/* ---------- Firmware Version ---------- */
#define FW_VERSION "1.0.0"

/* ---------- OTA Callback Functions ---------- */
void onOTAStart() {
  Serial.println("OTA Update Started");
}

void onOTAEnd() {
  Serial.println("\nOTA Update Finished");
}

void onOTAProgress(unsigned int progress, unsigned int total) {
  Serial.printf("OTA Progress: %u%%\r", (progress * 100) / total);
}

void onOTAError(ota_error_t error) {
  Serial.printf("OTA Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  else if (error == OTA_END_ERROR) Serial.println("End Failed");
}

void setup() {
  Serial.begin(115200);

  /* ---------- WiFi Connection ---------- */
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /* ---------- OTA Setup ---------- */
  ArduinoOTA.setHostname("ESP32_OTA_Device");

  ArduinoOTA.onStart(onOTAStart);
  ArduinoOTA.onEnd(onOTAEnd);
  ArduinoOTA.onProgress(onOTAProgress);
  ArduinoOTA.onError(onOTAError);

  ArduinoOTA.begin();

  Serial.println("OTA Ready");
  Serial.print("Firmware Version: ");
  Serial.println(FW_VERSION);
  Serial.println("OTA UPDATED");
}

void loop() {
  ArduinoOTA.handle();   // VERY IMPORTANT
}