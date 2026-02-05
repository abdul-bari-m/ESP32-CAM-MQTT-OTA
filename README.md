# ESP32-CAM Environmental Monitor (MQTT • OTA • Deep Sleep)

## Overview

This project uses an **ESP32-CAM** to read temperature and humidity from a **DHT11 sensor** and send the data securely to **HiveMQ Cloud** using MQTT.
The device supports **OTA (Over-The-Air) firmware updates** and uses **deep sleep** to save power when it’s not active.

Firmware versioning is included so OTA updates can be easily verified.

---

## What this project does

* Connects to Wi-Fi with timeout handling
* Publishes sensor data securely using MQTT (TLS)
* Reports device status (online, sleeping, offline)
* Supports OTA firmware updates without USB cable
* Enters deep sleep automatically to reduce power usage
* Prints firmware version at boot for OTA verification

---

## Hardware used

* ESP32-CAM
* DHT11 temperature & humidity sensor
* Onboard flash LED (GPIO 4)
* USB-TTL adapter (for first upload and debugging)

---

## MQTT setup

This project uses **one MQTT broker** with **two topics**.

### Sensor data topic

```
iot/esp32cam/env
```

Example message:

```json
{
  "device": "ESP32_CAM_ENV",
  "temp": 28.6,
  "hum": 61.2
}
```

### Status topic

```
iot/esp32cam/status
```

Status messages:

* `ONLINE` – device connected normally
* `SLEEPING` – device entered deep sleep
* `OFFLINE` – unexpected disconnect (Last Will message)

Keeping data and status in separate topics makes monitoring much easier.

---

## OTA update flow

1. ESP32 boots or wakes from deep sleep
2. Connects to Wi-Fi and MQTT
3. OTA service starts
4. Device stays awake for a fixed OTA window
5. If OTA begins, deep sleep is blocked
6. After OTA or timeout, device goes to deep sleep

At every boot, the firmware version is printed:

```
Firmware version: 1.0.2
```

This makes it easy to confirm that an OTA update worked.

---

## Power management

* Sensor data is sent **once per wake cycle**
* OTA window stays open for **60 seconds**
* Device then enters **deep sleep for 1 minute**
* Wi-Fi and MQTT are shut down cleanly before sleeping

This keeps power usage low while still allowing remote updates.

---

## Screenshots

### Hardware – Deep sleep state

LED is off while the device is in deep sleep.

![Hardware deep sleep](images/hardware_active_led_on.png)

---

### Hardware – Active and publishing

LED turns on when the device is awake and sending data.

![Hardware active](images/hardware_active_led_on.jpg)

---

### Serial monitor logs

Shows Wi-Fi connection, MQTT connection, and data publishing.

![Serial monitor logs](images/serial_monitor_wifi_mqtt_logs.jpg)

---

### OTA update via Arduino IDE

Firmware update in progress using OTA and New firmware version running after OTA update

![OTA update](images/ota_update_arduino_ide.jpg)

---

### MQTT messages in HiveMQ Cloud

Sensor data and status messages visible in HiveMQ web client.

![MQTT messages](images/mqtt_messages_hivemq_cloud.jpg)

---

## Firmware versioning

Firmware version is defined in code:

```cpp
#define FW_VERSION "1.0.2"
```

* Version is increased for every OTA update
* GitHub code and device firmware always match
* Makes testing and verification simple

---

## How to run

1. Update Wi-Fi and MQTT credentials in the code
2. Upload firmware once using USB
3. Open Serial Monitor to confirm connections
4. Perform OTA updates using Arduino IDE
5. View MQTT data using HiveMQ Cloud web client or MQTT Explorer

---

## Folder structure

```
ESP32-CAM-MQTT-OTA/
├── src/
│   └── main/
|       └── main.ino
├── images/
│   ├── hardware_deep_sleep_led_off.png
│   ├── hardware_active_led_on.png
│   ├── serial_monitor_wifi_mqtt_logs.jpg
│   ├── ota_update_arduino_ide.png
│   └── mqtt_messages_hivemq_cloud.png
└── README.md
```

---

## Summary

This project shows a practical way to build an IoT device using ESP32-CAM with secure MQTT communication, OTA firmware updates, and power-efficient deep sleep. It’s simple to understand, easy to maintain, and suitable for learning, demos, and small real-world use cases.

---
## Author
Abdul Bari  
IoT & Embedded Developer  

---
