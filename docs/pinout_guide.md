# Hardware Pinout & Wiring Guide

This document details the physical pin connections, electrical specifications, and wiring schematics for the **Smart Shoe for Blind Navigation System**.

---

## 1. Complete Pinout Table (Arduino Uno / ATmega328P)

| Arduino Uno Pin | Peripheral | Module Pin | Direction / Mode | Operating Specs | Functional Description |
|---|---|---|---|---|---|
| **Pin 9** | Front Ultrasonic | `TRIG` | Output (Digital) | 5V CMOS, 10µs pulse | Transmits 40 kHz ultrasonic burst |
| **Pin 8** | Front Ultrasonic | `ECHO` | Input (Digital) | 5V TTL | Measures pulse flight duration (Proximity) |
| **Pin 7** | Downward Ultrasonic | `TRIG` | Output (Digital) | 5V CMOS, 10µs pulse | Measures floor distance / drop-off |
| **Pin 6** | Downward Ultrasonic | `ECHO` | Input (Digital) | 5V TTL | Detects stairs down / open potholes |
| **Pin 5** | Haptic Vibration Motor | Base/Gate Driver | Output (PWM ~980Hz) | 0–5V (PWM Duty 0–255) | Drives ERM/LRA vibration disc via transistor |
| **Pin 3** | Piezo Buzzer | Positive Terminal | Output (PWM/Tone) | 1.8 kHz – 3.1 kHz | Acoustic proximity and alert pitch |
| **Pin 2** | Pushbutton | Contact Pin | Input (`INPUT_PULLUP`) | External Interrupt (INT0) | Toggles silent mode / audio mute |
| **Pin 12** | Hazard Indicator LED | Anode via 220Ω | Output (Digital) | ~15mA at 5V | Red LED indicates active hazard state |
| **Pin 13** | Status Heartbeat LED | Onboard LED | Output (Digital) | 1 Hz toggle | Visual heartbeat confirmation of firmware health |
| **Pin A0** | Water/Moisture Sensor | Signal / Analog Out | Input (ADC) | 0 – 1023 (10-bit ADC) | Detects puddles or standing water hazards |
| **5V** | Power Rail | VCC / +5V | Power Output | 500mA limit from USB/Regulator | Powers HC-SR04 sensors & sensor boards |
| **GND** | Ground Rail | GND / 0V | Ground reference | Common system ground | Return path for all peripherals |

---

## 2. Electrical Circuit Schematic Notes

### A. Haptic Vibration Motor Driver Circuit
> [!CAUTION]
> Never connect a vibration motor directly to an Arduino GPIO pin! Motors can draw 60–100mA and generate inductive back-EMF spikes that will destroy microcontroller I/O pins.

```text
       Arduino Pin 5 (PWM)
              |
           [ 1kΩ ]
              |
              +---- Base (NPN Transistor e.g. 2N2222 / BC547)
              |     (or Gate of N-Channel Logic MOSFET e.g. 2N7000)
              |
             --- Emitter -> Arduino GND
              
       +5V Rail
          |
          +--------+
          |        |
        [Motor]  [1N4001 Diode (Cathode to +5V, Anode to Collector)]
          |        |
          +--------+
          |
       Collector of NPN Transistor
```

### B. Piezo Buzzer Connection
- Connect buzzer positive pin to **Pin 3**.
- Connect buzzer negative pin to **GND** (or via a 100Ω current limiting resistor to soften audio volume if necessary).

### C. Water / Puddle Sensor
- Placed on the outer perimeter of the shoe sole.
- Connect VCC to **5V**, GND to **GND**, and A0/SIG to **Analog A0**.
- When dry: Resistance is very high, analog reading ~ 900–1023.
- When submerged or contacting wet puddle: Conductivity bridges traces, analog reading drops < 400.

### D. Power Supply Recommendations for Wearable Integration
1. **Rechargeable Battery**: 3.7V Li-Po (1200mAh – 2000mAh) with a 5V Step-Up (Boost) Converter module (e.g., MT3608 or TP4056 + Boost) supplying clean 5V to the Arduino 5V/VIN pin.
2. **Standard 9V Battery**: Connect to Arduino barrel jack or VIN and GND (acceptable for rapid prototyping).
