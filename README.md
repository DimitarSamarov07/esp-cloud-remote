# ❄️ Smart AC Control with ESP32 (ESP-IDF + IR + Web UI)

Control your air conditioner using an ESP32 via infrared (IR) signals and a web interface. This project uses **ESP-IDF** and supports both **sending and receiving IR codes**, allowing you to control and learn from physical remotes. You can access the AC controls from **any network** through a web dashboard or MQTT.

---

## 🚀 Features

- 🔹 Turn AC on/off
- 🔹 Set temperature, mode, fan speed
- 🔹 Receive and decode IR signals from any remote
- 🔹 Send saved IR codes to your AC
- 🔹 Host a web interface (on ESP32 or external server)
- 🔹 Remote access via MQTT

---

## 🧰 Hardware Requirements

A full part list (with links and quantities) is available in the [`Hardware/parts_list.txt`](Hardware/parts_list.txt) file.

### Core Components:

- ✅ ESP32 Development Board  
- ✅ IR LED (for sending commands)  
- ✅ IR Receiver Module (e.g., VS1838B)  
- ✅ NPN Transistor (e.g., 2N2222 or S8050)  
- ✅ Resistors (for IR LED and transistor base)  
- ✅ Breadboard & jumper wires  
- ✅ 5V USB Power Supply  

---

## 🌐 Web Interface

The ESP32 hosts a lightweight web dashboard for controlling the AC. From here, you can:

- Send power, temperature, and mode commands
- View current connection status
- Start **IR learning** to capture remote control codes
- Use it from **any device on any network** (via port forwarding or MQTT)

---

## 🛰️ IR Receiving Capabilities

This project includes an **IR decoder**.

You can:
- Point your AC remote at the IR receiver
- Capture raw IR codes
- Log and reuse them for future control
- Use the web UI to initiate IR learning

Perfect for supporting **any AC brand** using IR.

---

