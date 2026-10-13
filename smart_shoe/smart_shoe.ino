/**
 * @file smart_shoe.ino
 * @brief Smart Shoe for Blind Navigation - Arduino Main Firmware
 * 
 * Hardware Platform: Arduino Uno (ATmega328P)
 * Features:
 *  - Dual HC-SR04 ultrasonic sensors (front obstacle + downward cliff/pothole)
 *  - Water/Puddle detection probe
 *  - Non-blocking real-time scheduler
 *  - Outlier rejection (Median + Exponential Moving Average filtering)
 *  - Multi-tiered haptic vibrotactile feedback (PWM modulated)
 *  - Acoustic piezo buzzer alerting with mute toggle switch
 *  - Safety telemetry and heartbeat diagnostics over Serial
 */

#include "config.h"
#include "sensor_manager.h"
#include "feedback_manager.h"

// System Components
SensorManager sensors;
FeedbackManager feedback;

// Diagnostics & Timing
unsigned long lastTelemetryTime = 0;
unsigned long lastHeartbeatTime = 0;
bool heartbeatState = false;

// Interrupt / Button debounce state
volatile bool buttonPressedFlag = false;
unsigned long lastButtonInterruptTime = 0;

void onMuteButtonPressed() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastButtonInterruptTime > DEBOUNCE_DELAY_MS) {
        buttonPressedFlag = true;
        lastButtonInterruptTime = interruptTime;
    }
}

void printBanner() {
    Serial.println(F("=================================================="));
    Serial.println(F("    SMART SHOE FOR BLIND NAVIGATION SYSTEM       "));
    Serial.println(F("    Assistive Wearable Embedded System v1.0      "));
    Serial.println(F("=================================================="));
    Serial.println(F("[INFO] Initializing hardware peripherals..."));
}

void printTelemetry(const SensorReadings& r, unsigned long now) {
    Serial.print(F("[TIME: "));
    Serial.print(now / 1000);
    Serial.print(F("s] FRONT: "));
    Serial.print(r.frontDistanceCm, 1);
    Serial.print(F(" cm | DOWN: "));
    Serial.print(r.downDistanceCm, 1);
    Serial.print(F(" cm | WATER: "));
    Serial.print(r.waterDetected ? F("WET! [!] ") : F("DRY     "));
    Serial.print(F(" | DROP: "));
    Serial.print(r.dropOffDetected ? F("CLIFF! [!] ") : F("FLAT       "));
    Serial.print(F(" | SEV: "));
    
    switch (r.obstacleSeverity) {
        case SEVERITY_CRITICAL: Serial.print(F("CRITICAL")); break;
        case SEVERITY_WARNING:  Serial.print(F("WARNING ")); break;
        case SEVERITY_NOTICE:   Serial.print(F("NOTICE  ")); break;
        case SEVERITY_CLEAR:    Serial.print(F("CLEAR   ")); break;
    }

    Serial.print(F(" | AUDIO: "));
    Serial.println(feedback.isAudioMuted() ? F("MUTED") : F("ACTIVE"));
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000) {
        // Wait up to 2 seconds for USB CDC if applicable
    }

    printBanner();

    // Configure mute button with internal pull-up and interrupt
    pinMode(PIN_MUTE_BUTTON, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_MUTE_BUTTON), onMuteButtonPressed, FALLING);

    // Initialize sensor arrays and feedback actuators
    sensors.begin();
    feedback.begin();

    Serial.println(F("[SUCCESS] Calibration complete. Ready for navigation."));
    Serial.println(F("--------------------------------------------------"));
}

void loop() {
    unsigned long currentMillis = millis();

    // 1. Process Mute Button
    if (buttonPressedFlag) {
        buttonPressedFlag = false;
        feedback.toggleAudioMute();
        Serial.print(F("[EVENT] Audio Mute state changed -> "));
        Serial.println(feedback.isAudioMuted() ? F("MUTED (Vibration Only)") : F("ENABLED"));
    }

    // 2. Poll & Filter Sensors
    sensors.update(currentMillis);
    const SensorReadings& readings = sensors.getReadings();

    // 3. Update Haptic, Audio, and Visual Alerts
    feedback.update(readings, currentMillis);

    // 4. System Heartbeat LED (1 Hz toggle on Pin 13)
    if (currentMillis - lastHeartbeatTime >= 500) {
        lastHeartbeatTime = currentMillis;
        heartbeatState = !heartbeatState;
        digitalWrite(PIN_STATUS_LED, heartbeatState ? HIGH : LOW);
    }

    // 5. Periodic Serial Telemetry
    if (currentMillis - lastTelemetryTime >= SERIAL_TELEMETRY_MS) {
        lastTelemetryTime = currentMillis;
        printTelemetry(readings, currentMillis);
    }
}
