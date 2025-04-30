# ❄️ Smart AC Control with ESP32  
> 🛰️ IR Remote + 🌐 Web UI + ⚙️ ESP-IDF

Control your air conditioner from anywhere using an **ESP32**, **infrared (IR)**, and a simple **web interface**. This project allows you to **learn commands from your AC remote** and replay them via web or MQTT. Works across **any network**!

---

## ✨ Features

- 🟢 Power ON/OFF control  
- 🌡️ Temperature adjustment  
- 🌀 Fan speed & mode selection  
- 📥 IR **receiving** (learn AC remote codes)  
- 📤 IR **transmitting** (replay learned codes)  
- 🌐 Host a web UI on the ESP32  
- 📡 Optional MQTT support for remote control  

---

## 🧰 Hardware Requirements

Full part list available here:  
👉 [`Hardware/parts_list.md`](Hardware/parts_list.md)

| 🧩 Component         | 📋 Description                      |
|----------------------|--------------------------------------|
| ⚙️ ESP32 Board        | Main microcontroller                 |
| 📤 IR LED             | Sends IR commands to AC              |
| 📥 IR Receiver        | Captures IR signals from remote      |
| 🔌 NPN Transistor     | Drives the IR LED                    |
| 🧮 Resistors          | For limiting current                 |
| 🪛 Breadboard & Wires | For prototyping                      |
| 🔋 Power Supply       | USB 5V                               |

---

## 🌐 Web Interface

ESP32 hosts a sleek and mobile-friendly web UI:

- 🖱️ Send AC commands (Power, Temp, Mode, Fan)
- 📲 Use from phone, tablet, or PC
- 📥 Press a button to start **IR learning**
- 🌍 Accessible on **any network** (via port forwarding, MQTT, or DDNS)

---

## 🛰️ IR Receiving Capabilities

This project supports **IR learning** via an IR receiver connected to the ESP32's RMT peripheral.

- 📡 Capture IR codes from any remote
- 🔎 View the decoded signal in logs
- 💾 Store and reuse codes
- 🎛️ Build support for **MOST BRANDS** of AC

---
