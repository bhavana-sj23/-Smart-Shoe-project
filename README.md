# 🥾 Smart Shoe for Blind Navigation

> **An intelligent assistive-technology wearable powered by Arduino UNO, multi-sensor telemetry, and embedded C to enable safer, obstacle-free, and independent navigation for visually impaired individuals.**

[![Platform: Arduino Uno](https://img.shields.io/badge/Platform-Arduino%20Uno%20%2F%20ATmega328P-blue.svg)](https://www.arduino.cc/)
[![Language: Embedded C / C++](https://img.shields.io/badge/Language-Embedded%20C%20%2F%20C%2B%2B-orange.svg)](https://isocpp.org/)
[![Simulation: Wokwi Ready](https://img.shields.io/badge/Simulation-Wokwi%20Ready-brightgreen.svg)](https://wokwi.com)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## 📌 1. Project Overview & Assistive Motivation

Traditional assistive tools such as white canes provide physical tactile feedback but have critical limitations:
- Limited forward detection distance (~1 step ahead).
- Inability to preemptively detect **descending stairs, open manholes, or curbs**.
- Susceptibility to missing **puddles or slippery water bodies** that pose slip-and-fall hazards.
- Continuous wrist fatigue from sweeping.

The **Smart Shoe for Blind Navigation** addresses these challenges by transforming ordinary footwear into an active spatial awareness scanner. Utilizing forward and downward ultrasonic sensing combined with moisture detection, it delivers **multimodal alerts (vibrotactile haptic pulses and graduated audio beeps)** without requiring visual attention.

---

## ✨ 2. Key Features

- 🎯 **Dual Ultrasonic Spatial Scanning**:
  - **Front Sensor**: Detects knee-to-head obstacles (walls, vehicles, poles, people) up to **300 cm** away with graduated distance zones.
  - **Downward Sensor (45°)**: Identifies sudden elevation drops, open potholes, curbs, and staircases.
- 💧 **Puddle & Wet Ground Warning**: Sole-mounted conductive probe alerts the wearer to standing water and slippery puddles before stepping in.
- 📳 **Graduated Haptic Vibrotactile Feedback**:
  - Discreet, non-fatiguing vibration disc motor positioned on the shoe tongue or collar.
  - Modulates frequency and pulse width based on proximity to obstacles (Gentle pulses → Rapid flutter → Continuous emergency vibration).
- 🔊 **Acoustic Warning with Silent Mode**:
  - Pitch- and frequency-modulated piezo buzzer.
  - Hardware interrupt toggle button allows switching into **Haptic-Only Silent Mode** in public, meetings, or classrooms.
- 🛡️ **Gait Jitter & Ground Reflection Filtering**:
  - Hybrid **Sliding Median Filter** + **Exponential Moving Average (EMA)** algorithm cancels false triggers caused by normal leg swinging motion while walking.
- ⚡ **Dual Software Architectures Provided**:
  - **Modular Arduino C++ Framework** (`smart_shoe/`): Clean OOP structure with zero blocking delays.
  - **Bare-Metal AVR Embedded C** (`avr_baremetal/`): Pure register-level implementation (Timers, ADC, External Interrupts) for low power and high execution speed.
- 🌐 **Instant Simulation Ready**: Includes complete [diagram.json](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/diagram.json) and [wokwi.toml](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/wokwi.toml) configuration for immediate testing in Wokwi without physical hardware.

---

## 🗂️ 3. Repository Structure

```
Smart Shoe for Blind NaV/
├── README.md                      # Comprehensive project documentation
├── diagram.json                   # Wokwi simulation circuit layout (Smart Shoe)
├── wokwi.toml                     # Wokwi project configuration (Smart Shoe)
├── smart_shoe/                    # Modular Arduino C++ Firmware (Smart Shoe)
│   ├── smart_shoe.ino             # Main system entry point & non-blocking scheduler
│   ├── config.h                   # Pin definitions, thresholds & timing constants
│   ├── filter.h                   # Median & Exponential Moving Average filters
│   ├── sensor_manager.h           # Sensor telemetry interfaces
│   ├── sensor_manager.cpp         # Ultrasonic & water probe polling with crosstalk spacing
│   ├── feedback_manager.h         # Multimodal haptic & audio alert manager
│   └── feedback_manager.cpp       # PWM and tone pattern generator
├── avr_baremetal/                 # Pure Bare-Metal Embedded C (Smart Shoe)
│   ├── main.c                     # Direct ATmega328P register manipulation (No Arduino core)
│   └── Makefile                   # AVR-GCC compilation and AVRDUDE flashing script
├── power_anomaly_detection/       # IoT Power Anomaly & Theft Detection (ESP32 + Blynk)
│   ├── power_anomaly_detection.ino# ESP32 RMS Sampling, Impedance Z Calculation, Blynk Cloud
│   ├── README.md                  # Complete IoT setup, Blynk datastreams, and wiring guide
│   ├── diagram.json               # Wokwi ESP32 simulation schematic
│   └── wokwi.toml                 # Wokwi config for ESP32 project
└── docs/
    ├── circuit_diagram.md         # Circuit architecture & Mermaid state diagrams
    └── pinout_guide.md            # Hardware wiring & electrical specifications
```

---

## 🔌 4. Hardware Architecture & Bill of Materials (BOM)

### Bill of Materials

| Component | Part / Spec | Qty | Purpose |
|---|---|:---:|---|
| Microcontroller | Arduino Uno R3 (or ATmega328P Nano / Pro Mini) | 1 | Real-time processing and signal filtering |
| Forward Distance Sensor | HC-SR04 Ultrasonic Sensor | 1 | Forward path obstacle detection |
| Downward Distance Sensor | HC-SR04 Ultrasonic Sensor (or Sharp GP2Y0A21YK) | 1 | Drop-off, cliff, and pothole detection |
| Water / Rain Probe | Conductive PCB sensor plate / probe | 1 | Bottom sole puddle detection |
| Haptic Vibration Motor | 1027 Coin/Disc ERM Motor (3V–5V, 80mA) | 1 | Direct skin vibrotactile feedback |
| Transistor Driver | 2N2222 / BC547 NPN (or 2N7000 MOSFET) | 1 | Low-side motor switching driver |
| Diode | 1N4001 / 1N4148 Flyback Diode | 1 | Inductive kickback protection for motor |
| Audio Buzzer | 5V Piezo Buzzer (Active or Passive) | 1 | Acoustic alarm |
| Pushbutton | Tactile momentary switch | 1 | Audio Mute / Silent mode toggle |
| Resistors | 1 kΩ (base), 220 Ω (LEDs), 10 kΩ (pull-up) | 4 | Current limiting and biasing |
| LEDs | Red (Hazard), Green (Status) | 2 | Visual status diagnostics |
| Power Supply | 3.7V 18650 LiPo (1500mAh) + MT3608 5V Step-Up | 1 | Portable wearable power source |

### Pinout Mapping

| Arduino Pin | Peripheral Connection | Signal Type | Function |
|:---:|---|---|---|
| **D9** | Front Ultrasonic `TRIG` | Digital Output | 10 µs trigger pulse |
| **D8** | Front Ultrasonic `ECHO` | Digital Input | Echo pulse width measurement |
| **D7** | Downward Ultrasonic `TRIG` | Digital Output | 10 µs trigger pulse |
| **D6** | Downward Ultrasonic `ECHO` | Digital Input | Echo pulse width measurement |
| **D5** | Haptic Motor Transistor Base | PWM Output | Proportional vibration intensity |
| **D3** | Piezo Buzzer (+) | PWM / Tone | Pitch-modulated acoustic alarm |
| **D2** | Mute Pushbutton | Interrupt (INT0) | Toggle Silent Mode (Falling Edge) |
| **D12** | Red Hazard LED | Digital Output | Visual alert for bystanders |
| **D13** | Onboard Heartbeat LED | Digital Output | 1 Hz firmware liveness check |
| **A0** | Water Sensor Analog Output | Analog Input | Voltage drop on moisture contact |

---

## 🧠 5. Detection Zones & Alert Patterns

The firmware categorizes spatial telemetry into graduated risk tiers:

```
Distance:   0 cm                 35 cm                 80 cm                150 cm              300 cm
            |---------------------|---------------------|--------------------|-------------------|
Zone:       |    CRITICAL ZONE    |    WARNING ZONE     |    NOTICE ZONE     |    SAFE / CLEAR   |
Haptic:     | Continuous (100% PWM)| Rapid Pulse (100ms) | Soft Pulse (120ms) | OFF               |
Buzzer:     | Piercing (2.8 kHz)  | Fast Beeps (2.4 kHz)| Spaced Chirp (1.9k)| OFF               |
LED:        | Solid RED           | Solid RED           | Solid RED          | OFF               |
```

### Specialized Hazards
- **Cliff / Pothole Detected (`Down > 27 cm`)**: Urgent high-frequency double-stutter vibration + alternating dual-tone acoustic siren (3.1 kHz / 2.4 kHz).
- **Water / Puddle Detected (`ADC A0 < 400`)**: Rhythmic vibration rumble + gentle acoustic chirp warning of wet surface.

---

## 📈 6. Signal Processing & Gait Jitter Rejection

During normal walking, the human leg undergoes swing and stance phases, tilting the shoe by ±15° and causing transient distance fluctuations:

$$\text{Filtered Distance} = \text{EMA}\Big(\text{Median}\big(d_t, d_{t-1}, d_{t-2}, d_{t-3}, d_{t-4}\big)\Big)$$

1. **Sliding Median Window ($N=5$)**: Completely discards single-sample spikes caused by ground reflection multipath or acoustic bounce.
2. **Exponential Moving Average ($\alpha = 0.35$)**:
   $$S_t = \alpha \cdot M_t + (1 - \alpha) \cdot S_{t-1}$$
   Produces a smooth, responsive trajectory without sudden motor jerk.
3. **Acoustic Separation**: A 2000 µs guard interval between front and downward ultrasonic pulses eliminates inter-sensor acoustic crosstalk.

---

## 🚀 7. Quick Start & Flashing Guide

### Method A: Arduino IDE / Arduino CLI
1. Clone this repository:
   ```bash
   git clone "https://github.com/your-username/smart-shoe-blind-nav.git"
   cd "smart-shoe-blind-nav"
   ```
2. Open [smart_shoe.ino](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/smart_shoe/smart_shoe.ino) in the Arduino IDE.
3. Select board: **Tools > Board > Arduino AVR Boards > Arduino Uno**.
4. Select port: **Tools > Port > COMx**.
5. Click **Upload** (or use Arduino CLI):
   ```bash
   arduino-cli compile --fqbn arduino:avr:uno smart_shoe/
   arduino-cli upload -p COM3 --fqbn arduino:avr:uno smart_shoe/
   ```
6. Open the Serial Monitor at **115200 baud** to view real-time distance telemetry and hazard status.

### Method B: Bare-Metal AVR-GCC (Low Overhead)
If compiling without the Arduino framework:
```bash
cd avr_baremetal
make
make flash PORT=COM3
```

---

## 💻 8. Instant Simulation in Wokwi

You can test and simulate this project without any physical hardware:
1. Open [Wokwi](https://wokwi.com) in your web browser or install the **Wokwi Simulator** extension in VS Code.
2. Load [diagram.json](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/diagram.json) and [smart_shoe/smart_shoe.ino](file:///c:/Smart%20Shoe%20for%20Blind%20NaV/smart_shoe/smart_shoe.ino).
3. Click the **Play / Start Simulation** button.
4. Interact with the simulation:
   - Click the **Front Ultrasonic Sensor** to drag distance slider (<35cm triggers Critical alert).
   - Click the **Downward Sensor** to simulate approaching a curb or staircase.
   - Adjust the **Water Probe Potentiometer** to simulate wet ground.
   - Press the **Mute Pushbutton** to toggle audio on/off.

---

## 🛠️ 9. Calibration & Field Testing Checklist

1. **Bench Test (Stationary)**:
   - Power up the system; verify the onboard LED blinks at 1 Hz (Heartbeat).
   - Place an obstacle at 20 cm: Verify continuous vibration and 2.8 kHz buzzer.
   - Place an obstacle at 60 cm: Verify rapid pulsed vibration.
   - Hold shoe over table edge: Verify downward cliff alert activates.
2. **Moisture Test**:
   - Dampen a cloth and touch the water probe: Verify puddle rumble pattern triggers.
3. **Mute Function**:
   - Press the mute button: Ensure the buzzer is silenced while haptics remain fully operational.
4. **Wearable Walking Test**:
   - Mount sensor assembly on test shoe; walk on flat pavement to verify zero false positives from ground reflection.

---

## 🔮 10. Future Roadmap

- 📡 **BLE / Bluetooth 5.0 Integration**: Sync telemetry to an Android/iOS app for smartphone text-to-speech directions.
- 🗺️ **GPS Turn-by-Turn Guidance**: Dual-shoe setup (left shoe vibrates for left turn, right shoe for right turn).
- 🔋 **Piezoelectric Energy Harvesting**: Harvest mechanical energy from heel strikes to extend battery life.
- 👁️ **Edge Computer Vision**: Add TinyML camera module (e.g., ESP32-CAM) to identify specific objects (crosswalks, signs, doors).

---

## 📜 License
This project is released under the **MIT License**. Created as an open assistive-technology platform to empower independent mobility.
