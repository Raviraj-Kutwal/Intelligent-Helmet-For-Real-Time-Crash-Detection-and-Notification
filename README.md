# 🪖 Smart Helmet Accident Detection System

An IoT-based safety solution designed to detect motorcycle accidents in real-time. This system utilizes motion sensors and GPS tracking to automatically notify emergency contacts with precise location data via SMS.

---

## 🚀 Project Overview
This project integrates hardware and software to bridge the gap between an accident and emergency response. 

**Core Components:**
* **Arduino Nano:** Central processing unit.
* **MPU6050:** 6-axis Accelerometer + Gyroscope for impact sensing.
* **NEO-6M GPS Module:** Real-time latitude and longitude tracking.
* **SIM800 GSM Module:** Cellular communication for SMS alerts.

## 🧠 How It Works
1.  **Monitoring:** The MPU6050 continuously tracks motion data.
2.  **Baselines:** The system maintains a dynamic baseline of normal riding motion.
3.  **Detection:** If sudden acceleration/impact exceeds the safety threshold:
    * An accident is triggered.
    * The system locks the current GPS coordinates.
    * A Google Maps link is generated.
4.  **Alerting:** An automated SMS is sent to a predefined emergency number via the GSM module.

---

## 🧩 Hardware Components

| Component | Purpose |
| :--- | :--- |
| **Arduino Nano** | Main system controller |
| **MPU6050** | Detects impact via acceleration/gyro changes |
| **NEO-6M GPS** | Fetches precise real-time coordinates |
| **SIM800 GSM** | Sends SMS alerts to emergency contacts |
| **External Power** | Ensures stable operation of high-draw modules |

---

## 🔌 Wiring Connections

### 1️⃣ GPS (NEO-6M)
* **VCC** → 5V
* **GND** → GND
* **TX** → D4
* **RX** → D3

### 2️⃣ MPU6050 (I2C)
* **VCC** → 3.3V
* **GND** → GND
* **SDA** → A4
* **SCL** → A5

### 3️⃣ SIM800 GSM (Hardware Serial)
* **TX** → D0
* **RX** → D1
* **GND** → GND (Common ground is mandatory)

> [!WARNING]  
> **Disconnect the GSM module (D0/D1) during code upload** to avoid serial communication conflicts.

---

## ⚡ Power Configuration
* **Stability:** Use a 1000µF capacitor near the SIM800 module to handle current spikes.
* **Voltage:** SIM800 performs best with a dedicated 4V regulated supply.
* **Grounding:** Ensure all components share a **common ground**.

## 🧪 Detection Logic
The system uses a specific algorithm to prevent false positives:
* **Step 1:** Calculate acceleration magnitude.
* **Step 2:** Compare against a rolling baseline.
* **Step 3:** Validate with Gyroscope magnitude (checks for orientation change).
* **Step 4:** Trigger SMS and initiate a "Cooldown" period to prevent duplicate alerts.

---

## 🛠 Technologies Used
* **Language:** Embedded C++
* **Platform:** Arduino IDE
* **Protocols:** I2C, UART Serial Communication
* **Parsing:** NMEA GPS Data Parsing
* **Commands:** GSM AT Commands

## 🚧 Future Roadmap
* [ ] Local buzzer for bystander alerts.
* [ ] Multi-contact emergency SMS support.
* [ ] Helmet removal/strap detection.
* [ ] IoT Dashboard integration for cloud logging.

---

## 👨‍💻 Authors
Raviraj Kutwal
Aayush Bhandare
Tejas Modhave
