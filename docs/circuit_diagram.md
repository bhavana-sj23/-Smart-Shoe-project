# Circuit Architecture & System Block Diagram

## High-Level System Architecture

```mermaid
graph TD
    subgraph Sensing Peripherals
        US_FRONT["HC-SR04 Front Sensor\n(Obstacle Detection: 3-300 cm)"]
        US_DOWN["HC-SR04 Downward Sensor\n(Drop-off / Cliff / Pothole Detection)"]
        WATER["Conductive Water Probe\n(Puddle / Moisture Detection on Sole)"]
        BTN["Pushbutton (INT0)\n(Audio Mute / Silent Mode Toggle)"]
    end

    subgraph Processing Unit: ATmega328P / Arduino Uno
        FSM["Real-Time Loop Scheduler\n(Non-blocking millis)"]
        FILTER["Digital Filtering Layer\n(Running Median + Exponential Moving Average)"]
        DECISION["Hazard Classification Engine\n- Critical (<35cm)\n- Warning (35-80cm)\n- Notice (80-150cm)\n- Cliff (>27cm)\n- Wet Ground"]
    end

    subgraph Actuators & Feedback
        HAPTIC["Vibration Motor (Pin 5 PWM)\n(Proportional Tactile Pulses)"]
        BUZZER["Piezo Buzzer (Pin 3 Tone)\n(Pitch & Frequency Modulated)"]
        HAZARD_LED["Hazard Alert LED (Pin 12)\n(Visual Warning for Bystanders)"]
        STATUS_LED["System Heartbeat LED (Pin 13)\n(1 Hz Liveness Indicator)"]
        TELEMETRY["UART Serial Telemetry (115200 Baud)\n(Real-time Debugging & Diagnostics)"]
    end

    US_FRONT -->|Trig Pin 9 / Echo Pin 8| FILTER
    US_DOWN -->|Trig Pin 7 / Echo Pin 6| FILTER
    WATER -->|Analog A0| FILTER
    BTN -->|Interrupt Pin 2| FSM

    FILTER --> DECISION
    DECISION --> FSM

    FSM --> HAPTIC
    FSM --> BUZZER
    FSM --> HAZARD_LED
    FSM --> STATUS_LED
    FSM --> TELEMETRY
```

---

## Detailed Block Diagram of Feedback States

```mermaid
stateDiagram-v2
    [*] --> Standby_Clear

    Standby_Clear --> Notice_Zone : Obstacle 80cm - 150cm
    Notice_Zone --> Warning_Zone : Obstacle 35cm - 80cm
    Warning_Zone --> Critical_Zone : Obstacle < 35cm
    Critical_Zone --> Warning_Zone : Obstacle > 35cm
    Warning_Zone --> Standby_Clear : Obstacle > 150cm

    Standby_Clear --> Cliff_Emergency : Ground Drop > 27cm
    Notice_Zone --> Cliff_Emergency : Ground Drop > 27cm
    Warning_Zone --> Cliff_Emergency : Ground Drop > 27cm
    Cliff_Emergency --> Standby_Clear : Level Ground Restored

    Standby_Clear --> Water_Hazard : Wet probe detected
    Water_Hazard --> Standby_Clear : Dry probe restored
```

---

## Physical Shoe Mounting Layout

```text
       =========================================================
       |                   SMART SHOE TOPOLOGY                 |
       =========================================================

                 [Front HC-SR04: Facing Forward ~5° angle]
                               /=======\
                              |  Toe    |
                              |  Cap    |
                              \=========/
                               |       |
      [Downward HC-SR04:       |       |   [Pushbutton: Tongue Collar]
       Angled 45° downward]    | Tongue|   (Audio Mute Toggle)
                               |       |
                               |-------|
                               | Ankle |   [Vibration Disc Motor]
                               | Collar|   (Direct skin contact on ankle/tongue)
                               |_______|
                               |  Heel |   [Arduino Uno / Mini + Boost + LiPo]
                               \_______/   (Embedded in heel/external shoe clip)

      ---------------------------------------------------------
      SHOE SOLE BOTTOM:
      [Interlocking Copper Trace / Rain Sensor Plate] -> Ground & A0
```
