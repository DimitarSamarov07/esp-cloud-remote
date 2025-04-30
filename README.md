# 🌬️ Smart AC Control with ESP32 and ESP-IDF

Control your Air Conditioner remotely using an ESP32, ESP-IDF, and a web-based interface. This project enables IR-based or relay-based AC control, allowing temperature management and power toggling from any browser.

---

## 🚀 Features

- ✅ Remote AC control (ON/OFF, temperature, fan mode)
- 🌐 Web-based dashboard (mobile and desktop friendly)
- 📡 ESP32 with IR LED or relay module
- ⚙️ Built using ESP-IDF for low-level control
- 🔒 Wi-Fi and MQTT integration for remote access

---

## 🛠️ Hardware Requirements

| Component          | Description                         |
|-------------------|-------------------------------------|
| ESP32 Board        | e.g., ESP32 DevKit V1               |
| IR LED / Relay     | Depending on how the AC is controlled |
| Transistor + Resistor | For driving IR LED (if used)       |
| AC Unit            | That supports IR remote or relay    |
| Breadboard & Wires | For prototyping                     |
| Power Supply       | 5V USB or regulated source          |

---

## 🧰 Software Stack

- **ESP-IDF** (Espressif IoT Development Framework)
- **C/C++** for ESP32 firmware
- **HTML/CSS/JavaScript** for web interface
- **MQTT** for remote/cloud-based control

---

## 🌐 Web Interface

The control panel includes:

- Power ON/OFF switch
- Temperature up/down buttons
- Fan speed and mode selectors
- Status display from ESP32

Accessible from any device on the same network.

---


