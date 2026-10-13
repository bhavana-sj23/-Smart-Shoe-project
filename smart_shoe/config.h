/**
 * @file config.h
 * @brief Configuration constants, pin definitions, and calibration thresholds
 *        for Smart Shoe Blind Navigation System.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// PIN DEFINITIONS (Arduino Uno / ATmega328P)
// ============================================================================

// Front Ultrasonic Sensor (Obstacle detection at knee/waist level)
#define PIN_FRONT_TRIG       9   // Digital Output: Trigger pin for front sensor
#define PIN_FRONT_ECHO       8   // Digital Input:  Echo pin for front sensor

// Downward Ultrasonic Sensor (Pothole / Staircase drop-off / Cliff detection)
#define PIN_DOWN_TRIG        7   // Digital Output: Trigger pin for downward sensor
#define PIN_DOWN_ECHO        6   // Digital Input:  Echo pin for downward sensor

// Water / Puddle Sensor (Conductive probe / Rain sensor at sole)
#define PIN_WATER_SENSOR     A0  // Analog Input: Puddle detection probe

// Feedback Actuators
#define PIN_HAPTIC_MOTOR     5   // PWM Output: Transistor/MOSFET gate driving vibration motor
#define PIN_BUZZER           3   // PWM / Tone Output: Piezo buzzer

// User Interface & Indicators
#define PIN_MUTE_BUTTON      2   // Digital Input (Interrupt): Toggle audio alert / Silent mode
#define PIN_STATUS_LED       13  // Digital Output: Built-in LED / Operational status indicator
#define PIN_HAZARD_LED       12  // Digital Output: External visual warning indicator

// ============================================================================
// SYSTEM THRESHOLDS & CALIBRATION (Distances in centimeters)
// ============================================================================

// Forward Obstacle Distance Zones
#define DIST_FRONT_CRITICAL_CM   35    // < 35cm: Imminent collision -> Continuous intense alert
#define DIST_FRONT_WARNING_CM    80    // 35cm - 80cm: Approaching obstacle -> Rapid pulsing
#define DIST_FRONT_NOTICE_CM     150   // 80cm - 150cm: Distant obstacle -> Slow pulsing
#define DIST_FRONT_MAX_VALID_CM  300   // Upper range clamp (ignore echoes > 3m)
#define DIST_FRONT_MIN_VALID_CM  3     // Blind spot clamp (< 3cm)

// Downward Drop-off / Cliff Detection (Normal ground distance baseline ~ 10-15cm)
#define CLIFF_NORMAL_BASELINE_CM 12    // Nominal height of downward sensor to level ground
#define CLIFF_DROP_DELTA_CM      15    // Increase of > 15cm indicates pothole or downward stair drop
#define CLIFF_THRESHOLD_CM       (CLIFF_NORMAL_BASELINE_CM + CLIFF_DROP_DELTA_CM) // ~27cm

// Water / Puddle Detection
#define WATER_RAW_THRESHOLD      400   // ADC value below 400 (active low with pull-up) indicates moisture
#define WATER_HYSTERESIS         50    // Anti-chatter buffer for water detection

// ============================================================================
// TIMING CONSTANTS (Milliseconds)
// ============================================================================

#define SENSOR_POLL_INTERVAL_MS   50    // Ultrasonic sampling frequency (20 Hz)
#define WATER_POLL_INTERVAL_MS    200   // Water sensor sampling frequency (5 Hz)
#define DEBOUNCE_DELAY_MS         200   // Hardware button debounce period
#define SERIAL_TELEMETRY_MS       250   // Diagnostic logging cadence

// Filter buffer depth for sliding window / median filtering
#define FILTER_WINDOW_SIZE        5

#endif // CONFIG_H
