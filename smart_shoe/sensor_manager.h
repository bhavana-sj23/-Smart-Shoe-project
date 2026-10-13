/**
 * @file sensor_manager.h
 * @brief Interfaces for polling and filtering obstacle, drop-off, and water sensors.
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "filter.h"

enum ObstacleSeverity {
    SEVERITY_CLEAR = 0,
    SEVERITY_NOTICE,      // 80cm - 150cm
    SEVERITY_WARNING,     // 35cm - 80cm
    SEVERITY_CRITICAL     // < 35cm
};

struct SensorReadings {
    float frontDistanceCm;
    float downDistanceCm;
    int rawWaterValue;
    bool waterDetected;
    bool dropOffDetected;
    ObstacleSeverity obstacleSeverity;
};

class SensorManager {
private:
    MedianFilter<FILTER_WINDOW_SIZE> frontFilter;
    MedianFilter<FILTER_WINDOW_SIZE> downFilter;
    ExponentialFilter frontEma;
    ExponentialFilter downEma;

    SensorReadings currentReadings;
    unsigned long lastUltrasonicPoll;
    unsigned long lastWaterPoll;

    float measureDistanceCm(uint8_t trigPin, uint8_t echoPin, unsigned long timeoutMicros = 20000);

public:
    SensorManager();
    void begin();
    bool update(unsigned long currentMillis);
    const SensorReadings& getReadings() const;
};

#endif // SENSOR_MANAGER_H
