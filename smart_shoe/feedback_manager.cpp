/**
 * @file feedback_manager.cpp
 * @brief Dynamic cadence generation for haptic motor and piezo buzzer.
 */

#include "feedback_manager.h"

FeedbackManager::FeedbackManager()
    : audioMuted(false),
      lastPatternToggleTime(0),
      patternPhaseState(false) {
}

void FeedbackManager::begin() {
    pinMode(PIN_HAPTIC_MOTOR, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_STATUS_LED, OUTPUT);
    pinMode(PIN_HAZARD_LED, OUTPUT);

    analogWrite(PIN_HAPTIC_MOTOR, 0);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_STATUS_LED, LOW);
    digitalWrite(PIN_HAZARD_LED, LOW);

    // Initial power-up haptic confirmation click
    analogWrite(PIN_HAPTIC_MOTOR, 200);
    delay(80);
    analogWrite(PIN_HAPTIC_MOTOR, 0);
}

void FeedbackManager::toggleAudioMute() {
    audioMuted = !audioMuted;
    if (audioMuted) {
        noTone(PIN_BUZZER);
        digitalWrite(PIN_BUZZER, LOW);
    }
}

bool FeedbackManager::isAudioMuted() const {
    return audioMuted;
}

void FeedbackManager::updateVisuals(const SensorReadings& readings) {
    bool hazardActive = (readings.obstacleSeverity != SEVERITY_CLEAR) ||
                        readings.dropOffDetected ||
                        readings.waterDetected;

    digitalWrite(PIN_HAZARD_LED, hazardActive ? HIGH : LOW);
}

void FeedbackManager::updateHaptics(const SensorReadings& readings, unsigned long currentMillis) {
    // 1. Highest Priority Hazard: Drop-off / Cliff / Pothole (Immediate fall risk)
    if (readings.dropOffDetected) {
        // High frequency urgent stutter: 50ms ON, 50ms OFF
        if (currentMillis - lastPatternToggleTime >= 50) {
            lastPatternToggleTime = currentMillis;
            patternPhaseState = !patternPhaseState;
        }
        analogWrite(PIN_HAPTIC_MOTOR, patternPhaseState ? 255 : 0);
        return;
    }

    // 2. Water / Puddle Hazard: Distinctive gentle rumble (30ms ON, 120ms OFF)
    if (readings.waterDetected) {
        if (currentMillis - lastPatternToggleTime >= 100) {
            lastPatternToggleTime = currentMillis;
            patternPhaseState = !patternPhaseState;
        }
        analogWrite(PIN_HAPTIC_MOTOR, patternPhaseState ? 140 : 0);
        return;
    }

    // 3. Forward Obstacle Proximity
    switch (readings.obstacleSeverity) {
        case SEVERITY_CRITICAL:
            // Continuous high intensity vibration
            analogWrite(PIN_HAPTIC_MOTOR, 255);
            break;

        case SEVERITY_WARNING: {
            // Rapid pulse: 100ms ON, 100ms OFF
            unsigned long cycle = currentMillis % 200;
            if (cycle < 100) {
                analogWrite(PIN_HAPTIC_MOTOR, 200);
            } else {
                analogWrite(PIN_HAPTIC_MOTOR, 0);
            }
            break;
        }

        case SEVERITY_NOTICE: {
            // Gentle rhythmic pulse: 120ms ON, 480ms OFF
            unsigned long cycle = currentMillis % 600;
            if (cycle < 120) {
                analogWrite(PIN_HAPTIC_MOTOR, 150);
            } else {
                analogWrite(PIN_HAPTIC_MOTOR, 0);
            }
            break;
        }

        case SEVERITY_CLEAR:
        default:
            analogWrite(PIN_HAPTIC_MOTOR, 0);
            break;
    }
}

void FeedbackManager::updateBuzzer(const SensorReadings& readings, unsigned long currentMillis) {
    if (audioMuted) {
        noTone(PIN_BUZZER);
        digitalWrite(PIN_BUZZER, LOW);
        return;
    }

    // Drop-off warning: High-low alternating siren
    if (readings.dropOffDetected) {
        unsigned long cycle = currentMillis % 300;
        if (cycle < 150) {
            tone(PIN_BUZZER, 3100);
        } else {
            tone(PIN_BUZZER, 2400);
        }
        return;
    }

    // Water puddle detection: Short double clicks (every 800ms)
    if (readings.waterDetected) {
        unsigned long cycle = currentMillis % 800;
        if (cycle < 60 || (cycle >= 120 && cycle < 180)) {
            tone(PIN_BUZZER, 1800);
        } else {
            noTone(PIN_BUZZER);
        }
        return;
    }

    // Forward Obstacle Tone Cadence
    switch (readings.obstacleSeverity) {
        case SEVERITY_CRITICAL:
            // Piercing solid continuous tone
            tone(PIN_BUZZER, 2800);
            break;

        case SEVERITY_WARNING: {
            // Fast beeps: 80ms ON, 80ms OFF
            unsigned long cycle = currentMillis % 160;
            if (cycle < 80) {
                tone(PIN_BUZZER, 2400);
            } else {
                noTone(PIN_BUZZER);
            }
            break;
        }

        case SEVERITY_NOTICE: {
            // Low urgency spaced chirps: 70ms ON, 530ms OFF
            unsigned long cycle = currentMillis % 600;
            if (cycle < 70) {
                tone(PIN_BUZZER, 1900);
            } else {
                noTone(PIN_BUZZER);
            }
            break;
        }

        case SEVERITY_CLEAR:
        default:
            noTone(PIN_BUZZER);
            break;
    }
}

void FeedbackManager::update(const SensorReadings& readings, unsigned long currentMillis) {
    updateHaptics(readings, currentMillis);
    updateBuzzer(readings, currentMillis);
    updateVisuals(readings);
}
