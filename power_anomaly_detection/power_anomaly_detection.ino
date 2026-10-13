/**
 * @file power_anomaly_detection.ino
 * @brief IoT Power Anomaly & Theft Detection System using ESP32, Blynk IoT, and I2C LCD.
 * 
 * Technical Principles:
 *  - High-speed RMS sampling of Voltage (ZMPT101B) and Current (ACS712)
 *  - Real-time Line Impedance Calculation (Z = Vrms / Irms)
 *  - Power Theft / Tap Detection via Impedance Discrepancy & Threshold Logic
 *  - Non-blocking BlynkTimer scheduling to prevent Cloud disconnects
 *  - Visual reporting via 16x2 I2C LCD and Real-time Blynk IoT Cloud Dashboard
 */

// ============================================================================
// BLYNK CONFIGURATION
// ============================================================================
#define BLYNK_TEMPLATE_ID   "TMPL35--0-P5a"
#define BLYNK_TEMPLATE_NAME "power anomaly detection"
#define BLYNK_AUTH_TOKEN    "lwnSllXxbWFN9XxtVwJ6Tz2PUxOwr4Xy"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ============================================================================
// NETWORK CREDENTIALS
// ============================================================================
char ssid[] = "TEST24";
char pass[] = "12345678";

// ============================================================================
// HARDWARE PIN DEFINITIONS (ESP32)
// ============================================================================
#define CURRENT_PIN 34    // ADC1_CH6 (Input-only, ideal for sensor ADC)
#define VOLTAGE_PIN 35    // ADC1_CH7 (Input-only, ideal for sensor ADC)
#define BUZZER_PIN  25    // Digital GPIO for active buzzer alert

// ============================================================================
// PERIPHERALS & CALIBRATION CONSTANTS
// ============================================================================
// LCD I2C Address (typically 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Non-blocking timer for Blynk Cloud compliance
BlynkTimer timer;

// Calibration scale factors
// ZMPT101B / Voltage Divider scaling
const float voltageScale  = 0.0063f;

// ACS712 Current sensor mid-point offset (ESP32 12-bit ADC mid-scale ~ 2048 count)
const float currentOffset = 2050.0f;
const float currentScale  = 0.0005f;

// Anomaly / Theft Detection Thresholds
const float thresholdZ    = 25.0f;   // Expected normal grid impedance threshold
const float minCurrentA   = 0.30f;   // Minimum significant current to evaluate theft
const int   noiseFilterLimit = 3;    // Consecutive anomalous cycles needed before triggering alarm

// Operational Runtime State
bool alertSent   = false;
int  lowZCount   = 0;

// Cached metrics
float currentVrms = 0.0f;
float currentIrms = 0.0f;
float currentZ    = 0.0f;
bool  theftState  = false;

// ============================================================================
// RMS SENSING FUNCTIONS
// ============================================================================

/**
 * @brief Computes true RMS across 200 samples over a full AC cycle (50Hz / 60Hz).
 * @param pin ESP32 ADC pin to sample.
 * @return float Root-Mean-Square raw ADC value.
 */
float readRMS(int pin) {
    float sum = 0.0f;
    const int samples = 200;

    // ESP32 ADC reads 12-bit (0 - 4095)
    for (int i = 0; i < samples; i++) {
        float value = (float)analogRead(pin);
        sum += value * value;
        delayMicroseconds(200); // 200us * 200 = 40ms (2 full 50Hz cycles)
    }

    return sqrt(sum / (float)samples);
}

/**
 * @brief Computes true AC current RMS subtracting quiescent DC offset.
 */
float readCurrentRMS(int pin, float offset) {
    float sumSquares = 0.0f;
    const int samples = 200;

    for (int i = 0; i < samples; i++) {
        float value = (float)analogRead(pin);
        float delta = value - offset;
        sumSquares += delta * delta;
        delayMicroseconds(200);
    }

    return sqrt(sumSquares / (float)samples);
}

// ============================================================================
// CORE MONITORING & THEFT LOGIC
// ============================================================================

void processPowerMonitoring() {
    // 1. Acquire raw signals
    float rawV = readRMS(VOLTAGE_PIN);
    
    // AC current RMS calculation using midpoint offset
    float rawIAC = readCurrentRMS(CURRENT_PIN, currentOffset);
    float Irms = rawIAC * currentScale;

    // Voltage calculation
    float Vrms = rawV * voltageScale;

    // Prevent divide-by-zero or micro-noise currents
    if (Irms < 0.01f) {
        Irms = 0.01f;
    }

    // Dynamic Line Impedance (Ohm's Law: Z = V / I)
    float Z = Vrms / Irms;

    // Cache values
    currentVrms = Vrms;
    currentIrms = Irms;
    currentZ    = Z;

    // 2. Anomaly / Theft Evaluation
    // An abnormal impedance signature under load indicates unauthorized parallel load / tap
    if (Z > thresholdZ && Irms > minCurrentA) {
        lowZCount++;
    } else {
        lowZCount = 0;
    }

    // Multi-cycle debounce to reject transient switching surges
    if (lowZCount >= noiseFilterLimit) {
        theftState = true;
        digitalWrite(BUZZER_PIN, HIGH);

        if (!alertSent) {
            Serial.println(F("[ALERT] Power Theft Detected! Logging event to Blynk..."));
            Blynk.logEvent("theft_alert", "Power Theft Detected!");
            alertSent = true;
        }
    } else {
        theftState = false;
        digitalWrite(BUZZER_PIN, LOW);
        alertSent = false;
    }

    // 3. Serial Monitor Diagnostics
    Serial.print(F("V: "));
    Serial.print(Vrms, 1);
    Serial.print(F(" V | I: "));
    Serial.print(Irms, 3);
    Serial.print(F(" A | Z: "));
    Serial.print(Z, 2);
    Serial.print(F(" Ohm | Status: "));
    Serial.println(theftState ? F("THEFT DETECTED [!]") : F("NORMAL"));

    // 4. Update Blynk IoT Cloud Datastreams
    if (Blynk.connected()) {
        Blynk.virtualWrite(V0, Vrms);
        Blynk.virtualWrite(V1, Irms);
        Blynk.virtualWrite(V2, Z);
        Blynk.virtualWrite(V3, theftState ? 1 : 0);
    }

    // 5. Update 16x2 I2C LCD Display
    lcd.setCursor(0, 0);
    lcd.print("V:");
    lcd.print(Vrms, 1);
    lcd.print(" I:");
    lcd.print(Irms, 2);
    lcd.print("    ");

    lcd.setCursor(0, 1);
    if (theftState) {
        lcd.print("THEFT DETECTED! ");
    } else {
        lcd.print("SAFE Z:");
        lcd.print(Z, 1);
        lcd.print("    ");
    }
}

// ============================================================================
// SYSTEM SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println(F("=================================================="));
    Serial.println(F("     ESP32 POWER ANOMALY & THEFT DETECTOR         "));
    Serial.println(F("=================================================="));

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Configure ESP32 ADC resolution (12-bit: 0 - 4095)
    analogReadResolution(12);

    // Initialize 16x2 I2C LCD
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("POWER MONITOR");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    delay(1500);

    // WiFi Setup
    Serial.print(F("Connecting to WiFi: "));
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(500);
    WiFi.begin(ssid, pass);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");

    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - wifiStart < 15000)) {
        delay(500);
        Serial.print('.');
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n[SUCCESS] WiFi Connected!"));
        Serial.print(F("IP Address: "));
        Serial.println(WiFi.localIP());

        lcd.setCursor(0, 1);
        lcd.print("WiFi Connected! ");
    } else {
        Serial.println(F("\n[WARNING] WiFi Connection Timeout. Running in offline mode."));
        lcd.setCursor(0, 1);
        lcd.print("WiFi Timeout    ");
    }
    delay(1000);

    // Blynk Cloud Connection Setup
    Blynk.config(BLYNK_AUTH_TOKEN, "blynk.cloud", 80);
    if (WiFi.status() == WL_CONNECTED) {
        if (Blynk.connect(5000)) {
            Serial.println(F("[SUCCESS] Blynk Cloud Connected!"));
        } else {
            Serial.println(F("[ERROR] Blynk Cloud Connection Failed."));
        }
    }

    lcd.clear();

    // Schedule power monitoring routine to run every 1000ms via BlynkTimer
    // (Prevents loop blocking and cloud disconnects)
    timer.setInterval(1000L, processPowerMonitoring);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    // Keep Blynk connection alive
    if (WiFi.status() == WL_CONNECTED) {
        if (Blynk.connected()) {
            Blynk.run();
        } else {
            static unsigned long lastReconnectAttempt = 0;
            if (millis() - lastReconnectAttempt > 10000) {
                lastReconnectAttempt = millis();
                Serial.println(F("Attempting Blynk reconnection..."));
                Blynk.connect();
            }
        }
    }

    // Run scheduled timer tasks (Measurement, Analysis, Display update)
    timer.run();
}
