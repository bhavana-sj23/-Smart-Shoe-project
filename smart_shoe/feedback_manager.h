/**
 * @file feedback_manager.h
 * @brief Multimodal alerting engine combining vibrotactile (haptic), audio (buzzer),
 *        and visual signaling for intuitive assistive navigation.
 */

#ifndef FEEDBACK_MANAGER_H
#define FEEDBACK_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "sensor_manager.h"

class FeedbackManager {
private:
    bool audioMuted;
    unsigned long lastPatternToggleTime;
    bool patternPhaseState;

    void updateHaptics(const SensorReadings& readings, unsigned long currentMillis);
    void updateBuzzer(const SensorReadings& readings, unsigned long currentMillis);
    void updateVisuals(const SensorReadings& readings);

public:
    FeedbackManager();
    void begin();
    void update(const SensorReadings& readings, unsigned long currentMillis);
    void toggleAudioMute();
    bool isAudioMuted() const;
};

#endif // FEEDBACK_MANAGER_H
