# ⚡ IoT Power Anomaly & Theft Detection System

> **An ESP32-powered IoT electrical grid monitoring device utilizing root-mean-square (RMS) sampling, dynamic line impedance ($Z = V / I$) calculation, and the Blynk IoT Cloud to detect unauthorized electricity taps and power anomalies in real time.**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20DevKit%20V1-red.svg)](https://espressif.com/)
[![Framework: Arduino / ESP-IDF](https://img.shields.io/badge/Framework-Arduino%20Core-blue.svg)](https://github.com/espressif/arduino-esp32)
[![IoT: Blynk IoT](https://img.shields.io/badge/Cloud-Blynk%20IoT%20Cloud-green.svg)](https://blynk.io/)
[![Display: 16x2 I2C LCD](https://img.shields.io/badge/Display-I2C%20LCD%2016x2-orange.svg)](https://playground.arduino.cc/Main/LiquidCrystal/)
[![Simulation: Wokwi Ready](https://img.shields.io/badge/Simulation-Wokwi%20Ready-brightgreen.svg)](https://wokwi.com)

---

## 📌 1. Project Overview & Working Principle

Electricity theft through illegal line hooking and bypass taps causes massive commercial losses and severe electrical fire hazards. Conventional static meters only log consumed power at the endpoint, failing to detect upstream diversion.

This system detects power theft using **Line Impedance Anomaly Detection**:

$$Z = \frac{V_{\text{rms}}}{I_{\text{rms}}}$$

1. **Continuous RMS Sampling**: 
   - Voltage ($V_{\text{rms}}$) is continuously sampled via an AC voltage transformer/sensor (e.g., ZMPT101B or transformer module).
   - Current ($I_{\text{rms}}$) is sampled via a Hall-effect current sensor (e.g., ACS712-05B / 20A / 30A).
2. **Impedance Computation**: The microcontroller calculates the apparent grid impedance $Z$.
3. **Theft & Anomaly Condition**:
   - If an unauthorized tap/bypass is connected or impedance shifts beyond normal operating bounds ($Z > Z_{\text{threshold}}$ while carrying active current $I > 0.3\text{ A}$):
   - A multi-cycle noise debounce counter ($N \ge 3$) confirms the condition, rejecting brief motor startup surges.
4. **Instant Action & Multimodal Alerting**:
   - **Local Actuation**: Pin 25 triggers a piezo alarm buzzer.
   - **Local Display**: 16x2 I2C LCD displays `"THEFT DETECTED!"` along with live $V$ and $I$.
   - **Cloud Notification**: Pushes an instant mobile notification via `Blynk.logEvent("theft_alert")` and updates real-time telemetry graphs on the Blynk IoT dashboard.

---

## 🔌 2. Hardware Pinout & Wiring

| ESP32 Pin | Connected Peripheral | Peripheral Pin | Type | Notes |
|:---:|---|---|:---:|---|
| **GPIO 35** | Voltage Sensor (ZMPT101B) | Analog OUT / SIG | Input (ADC1_CH7) | Input-only pin, high linearity |
| **GPIO 34** | Current Sensor (ACS712) | Analog OUT / SIG | Input (ADC1_CH6) | Input-only pin, 2.5V mid-scale quiescent |
| **GPIO 25** | Piezo Buzzer / Relay | Positive (+) / Base | Output (Digital) | 3.3V logic to transistor/buzzer |
| **GPIO 21** | 16x2 I2C LCD Module | `SDA` | I2C Data | Default hardware I2C data bus |
| **GPIO 22** | 16x2 I2C LCD Module | `SCL` | I2C Clock | Default hardware I2C clock bus |
| **5V / VIN** | LCD & Sensors | `VCC` | Power (5V) | Power rail for sensors and LCD backlight |
| **GND** | All Peripherals | `GND` | Ground | Common reference ground |

---

## ☁️ 3. Blynk IoT Cloud Configuration

### Template Settings
- **Template ID**: `TMPL35--0-P5a`
- **Template Name**: `power anomaly detection`
- **Hardware**: `ESP32`
- **Connection Type**: `WiFi`

### Datastreams Setup
| Pin | Name | Data Type | Units | Min | Max |
|:---:|---|:---:|:---:|:---:|:---:|
| **V0** | Voltage ($V_{\text{rms}}$) | Double | `V` | 0 | 300 |
| **V1** | Current ($I_{\text{rms}}$) | Double | `A` | 0 | 30 |
| **V2** | Impedance ($Z$) | Double | `Ω` | 0 | 1000 |
| **V3** | Theft Status | Integer | – | 0 | 1 |

### Event Notification Setup
- **Event Name**: `theft_alert`
- **Event Description**: `Power Theft Detected!`
- **Type**: `Warning` / `Critical`
- **Action**: Enable Push Notification & In-App Banner

---

## ⚙️ 4. Key Firmware Enhancements

The included firmware in [`power_anomaly_detection.ino`](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/power_anomaly_detection/power_anomaly_detection.ino) incorporates critical embedded & IoT best practices:

1. **Non-Blocking Architecture with `BlynkTimer`**:
   - Eliminates standard blocking `delay(1000)` calls inside `loop()`.
   - Prevents heartbeat timeouts and frequent WiFi/Blynk Cloud disconnects.
2. **AC Symmetrical RMS Calculation**:
   - `readCurrentRMS()` subtracts the 2.5V / 2050 ADC zero-current quiescent midpoint before squaring, producing true sinusoidal RMS current.
3. **Automatic WiFi Reconnect Resilience**:
   - Periodic reconnection state machine keeps the offline monitoring running even if network connection drops.
4. **Multi-Cycle Noise Debouncing**:
   - Requires 3 consecutive abnormal sample cycles before asserting an alarm, preventing false alarms from inductive motor startup spikes.

---

## 🚀 5. How to Run

### Physical Hardware:
1. Open [`power_anomaly_detection/power_anomaly_detection.ino`](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/power_anomaly_detection/power_anomaly_detection.ino) in the Arduino IDE.
2. Install the required libraries via Arduino Library Manager:
   - `Blynk` by Volodymyr Shymanskyy
   - `LiquidCrystal_I2C` by Frank de Brabander
3. Update your WiFi SSID and Password in the sketch.
4. Select board **ESP32 Dev Module** and flash.
5. Open Serial Monitor at **115200 baud**.

### Wokwi Simulation:
1. Open the [diagram.json](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/power_anomaly_detection/diagram.json) in Wokwi.
2. Use the **Voltage Potentiometer (GPIO 35)** and **Current Potentiometer (GPIO 34)** to dynamically adjust $V$ and $I$.
3. When $Z > 25\ \Omega$ and $I > 0.3\text{ A}$, observe the buzzer sounding and LCD reporting `"THEFT DETECTED!"`.
