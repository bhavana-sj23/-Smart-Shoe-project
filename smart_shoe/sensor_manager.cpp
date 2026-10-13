/**
 * @file sensor_manager.cpp
 * @brief Implementation of ultrasonic sensor pulsing, cliff detection, and puddle measurement.
 */

#include "sensor_manager.h"

SensorManager::SensorManager()
    : lastUltrasonicPoll(0), lastWaterPoll(0) {
    currentReadings.frontDistanceCm = 400.0f;
    currentReadings.downDistanceCm = CLIFF_NORMAL_BASELINE_CM;
    currentReadings.rawWaterValue = 1023;
    currentReadings.waterDetected = false;
    currentReadings.dropOffDetected = false;
    currentReadings.obstacleSeverity = SEVERITY_CLEAR;
}

void SensorManager::begin() {
    pinMode(PIN_FRONT_TRIG, OUTPUT);
    pinMode(PIN_FRONT_ECHO, INPUT);
    pinMode(PIN_DOWN_TRIG, OUTPUT);
    pinMode(PIN_DOWN_ECHO, INPUT);

    digitalWrite(PIN_FRONT_TRIG, LOW);
    digitalWrite(PIN_DOWN_TRIG, LOW);

    frontFilter.reset(200.0f);
    downFilter.reset(CLIFF_NORMAL_BASELINE_CM);
    frontEma.reset(200.0f);
    downEma.reset(CLIFF_NORMAL_BASELINE_CM);
}

float SensorManager::measureDistanceCm(uint8_t trigPin, uint8_t echoPin, unsigned long timeoutMicros) {
    // Ensure trigger is low first
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Emit 10 microsecond ultrasonic pulse
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo high duration
    unsigned long duration = pulseIn(echoPin, HIGH, timeoutMicros);

    if (duration == 0) {
        // Echo timed out: obstacle either out of range (> 3.4m) or absorbed
        return DIST_FRONT_MAX_VALID_CM;
    }

    // Speed of sound in dry air at 20°C is ~343 m/s = 0.0343 cm/us
    // Distance = (time * speed) / 2
    float distance = (duration * 0.0343f) / 2.0f;

    // Constrain to physically realistic bounds
    if (distance < DIST_FRONT_MIN_VALID_CM) {
        distance = DIST_FRONT_MIN_VALID_CM;
    } else if (distance > DIST_FRONT_MAX_VALID_CM) {
        distance = DIST_FRONT_MAX_VALID_CM;
    }

    return distance;
}

bool SensorManager::update(unsigned long currentMillis) {
    bool updated = false;

    // Poll ultrasonic sensors at configured cadence
    if (currentMillis - lastUltrasonicPoll >= SENSOR_POLL_INTERVAL_MS) {
        lastUltrasonicPoll = currentMillis;

        // 1. Measure Front Obstacle
        float rawFront = measureDistanceCm(PIN_FRONT_TRIG, PIN_FRONT_ECHO, 18000);
        float medianFront = frontFilter.update(rawFront);
        currentReadings.frontDistanceCm = frontEma.update(medianFront);

        // Stagger the two ultrasonic sensors by a brief 2ms gap to prevent acoustic crosstalk
        delayMicroseconds(2000);

        // 2. Measure Downward Ground Drop
        float rawDown = measureDistanceCm(PIN_DOWN_TRIG, PIN_DOWN_ECHO, 8000);
        float medianDown = downFilter.update(rawDown);
        currentReadings.downDistanceCm = downEma.update(medianDown);

        // Classify obstacle zone
        if (currentReadings.frontDistanceCm <= DIST_FRONT_CRITICAL_CM) {
            currentReadings.obstacleSeverity = SEVERITY_CRITICAL;
        } else if (currentReadings.frontDistanceCm <= DIST_FRONT_WARNING_CM) {
            currentReadings.obstacleSeverity = SEVERITY_WARNING;
        } else if (currentReadings.frontDistanceCm <= DIST_FRONT_NOTICE_CM) {
            currentReadings.obstacleSeverity = SEVERITY_NOTICE;
        } else {
            currentReadings.obstacleSeverity = SEVERITY_CLEAR;
        }

        // Classify drop-off / pothole hazard
        if (currentReadings.downDistanceCm >= CLIFF_THRESHOLD_CM) {
            currentReadings.dropOffDetected = true;
        } else {
            currentReadings.dropOffDetected = false;
        }

        updated = true;
    }

    // Poll water/puddle sensor at its cadence
    if (currentMillis - lastWaterPoll >= WATER_POLL_INTERVAL_MS) {
        lastWaterPoll = currentMillis;

        currentReadings.rawWaterValue = analogRead(PIN_WATER_SENSOR);

        // Active low detection logic with hysteresis
        if (currentReadings.rawWaterValue < (WATER_RAW_THRESHOLD - WATER_HYSTERESIS)) {
            currentReadings.waterDetected = true;
        } else if (currentReadings.rawWaterValue > (WATER_RAW_THRESHOLD + WATER_HYSTERESIS)) {
            currentReadings.waterDetected = false;
        }

        updated = true;
    }

    return updated;
}

const SensorReadings& SensorManager::getReadings() const {
    return currentReadings;
}
