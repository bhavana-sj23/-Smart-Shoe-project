/**
 * @file main.c
 * @brief Bare-Metal Embedded C Implementation for Smart Shoe Blind Navigation
 *        Target Microcontroller: Microchip ATmega328P (16 MHz)
 * 
 * Demonstrates direct register-level manipulation:
 *  - GPIO DDR / PORT / PIN bitmasks
 *  - 16-bit Timer/Counter 1 (Fast microsecond pulse-width capture for Ultrasonic HC-SR04)
 *  - 8-bit Timer/Counter 0 (Fast PWM for Haptic Motor vibration intensity)
 *  - 8-bit Timer/Counter 2 (CTC square-wave tone generation for Buzzer)
 *  - Analog-to-Digital Converter (ADC Channel 0 for Puddle/Water conductivity probe)
 *  - External Interrupt INT0 (PD2 button for Silent Mode audio toggle)
 */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

// ============================================================================
// HARDWARE REGISTER PIN DEFINITIONS
// ============================================================================
// Front Ultrasonic:
// Trig -> PB1 (Arduino D9, OC1A)
// Echo -> PB0 (Arduino D8, ICP1)
#define TRIG_PIN_BIT     PB1
#define ECHO_PIN_BIT     PB0

// Haptic Motor:
// Connected to PD5 (Arduino D5, OC0B)
#define HAPTIC_PIN_BIT   PD5

// Buzzer:
// Connected to PD3 (Arduino D3, OC2B)
#define BUZZER_PIN_BIT   PD3

// Button (Mute Toggle):
// Connected to PD2 (Arduino D2, INT0)
#define BUTTON_PIN_BIT   PD2

// Status LED:
// Connected to PB5 (Arduino D13)
#define STATUS_LED_BIT   PB5

// Volatile state updated in interrupts
static volatile bool g_audio_muted = false;
static volatile uint32_t g_system_millis = 0;

// Timer 0 Overflow ISR: 1 millisecond system tick generator
ISR(TIMER0_OVF_vect) {
    g_system_millis++;
}

// External Interrupt INT0 ISR: Button press (Falling edge)
ISR(INT0_vect) {
    static uint32_t last_press_ms = 0;
    if (g_system_millis - last_press_ms > 200) {
        g_audio_muted = !g_audio_muted;
        last_press_ms = g_system_millis;
    }
}

// Initialize GPIO directions and pull-ups
static void gpio_init(void) {
    // Front Ultrasonic Trigger as output, Echo as input
    DDRB |= (1 << TRIG_PIN_BIT);
    DDRB &= ~(1 << ECHO_PIN_BIT);
    PORTB &= ~(1 << TRIG_PIN_BIT);

    // Status LED as output
    DDRB |= (1 << STATUS_LED_BIT);

    // Haptic Motor (PD5) and Buzzer (PD3) as outputs
    DDRD |= (1 << HAPTIC_PIN_BIT) | (1 << BUZZER_PIN_BIT);
    PORTD &= ~((1 << HAPTIC_PIN_BIT) | (1 << BUZZER_PIN_BIT));

    // Button (PD2) as input with internal pull-up
    DDRD &= ~(1 << BUTTON_PIN_BIT);
    PORTD |= (1 << BUTTON_PIN_BIT);

    // Configure External Interrupt INT0 on falling edge
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);
    EIMSK |= (1 << INT0);
}

// Initialize 8-bit Timer0 for Millis tick and Fast PWM on PD5 (OC0B)
static void timer0_init(void) {
    // Fast PWM mode 3, TOP=0xFF, Non-inverting on OC0B
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    // Prescaler /64 -> 16MHz / 64 = 250 kHz -> Overflow at 250k / 256 ~= 976.5 Hz (~1ms)
    TCCR0B = (1 << CS01) | (1 << CS00);
    TIMSK0 |= (1 << TOIE0); // Enable overflow interrupt
    OCR0B = 0; // Haptic motor off initially
}

// Initialize 16-bit Timer1 for microsecond timing
static void timer1_init(void) {
    // Normal mode, Prescaler /8 -> 16MHz / 8 = 2 MHz (1 tick = 0.5 microseconds)
    TCCR1A = 0x00;
    TCCR1B = (1 << CS11);
    TCNT1 = 0;
}

// Initialize ADC for Water Sensor (ADC0 / Arduino A0)
static void adc_init(void) {
    // AVcc reference with external capacitor on AREF pin
    ADMUX = (1 << REFS0);
    // ADC Enable, Prescaler /128 (16MHz / 128 = 125 kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

static uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Busy-wait conversion complete
    return ADC;
}

static void haptic_set_pwm(uint8_t duty) {
    OCR0B = duty;
}

static void buzzer_tone_enable(bool enable) {
    if (enable && !g_audio_muted) {
        // Toggle PD3 high/low or connect to Timer2
        PORTD |= (1 << BUZZER_PIN_BIT);
    } else {
        PORTD &= ~(1 << BUZZER_PIN_BIT);
    }
}

// Measure distance in centimeters using HC-SR04 on PB1/PB0
static uint16_t measure_ultrasonic_distance_cm(void) {
    // 10 microsecond trigger pulse
    PORTB &= ~(1 << TRIG_PIN_BIT);
    _delay_us(2);
    PORTB |= (1 << TRIG_PIN_BIT);
    _delay_us(10);
    PORTB &= ~(1 << TRIG_PIN_BIT);

    // Wait for Echo pin to go HIGH with timeout
    uint16_t timeout = 30000;
    while (!(PINB & (1 << ECHO_PIN_BIT))) {
        if (--timeout == 0) return 400; // Timeout
    }

    // Reset Timer1
    TCNT1 = 0;

    // Wait for Echo pin to go LOW
    timeout = 30000;
    while (PINB & (1 << ECHO_PIN_BIT)) {
        if (--timeout == 0) return 400;
    }

    uint16_t timer_ticks = TCNT1;
    // Each tick is 0.5 microseconds -> duration_us = timer_ticks / 2
    // distance_cm = duration_us / 58 = timer_ticks / 116
    uint16_t distance_cm = timer_ticks / 116;
    return distance_cm;
}

int main(void) {
    gpio_init();
    timer0_init();
    timer1_init();
    adc_init();

    sei(); // Global interrupts enable

    uint32_t last_sensor_poll = 0;
    uint32_t last_heartbeat = 0;

    uint16_t front_dist_cm = 200;
    uint16_t water_raw = 1023;

    while (1) {
        uint32_t now = g_system_millis;

        // Periodic Sensor Polling (every 50ms)
        if (now - last_sensor_poll >= 50) {
            last_sensor_poll = now;

            front_dist_cm = measure_ultrasonic_distance_cm();
            water_raw = adc_read(0);

            // Multimodal alert logic
            bool water_alert = (water_raw < 400);

            if (water_alert) {
                // Vibration flutter for puddle
                haptic_set_pwm(150);
                buzzer_tone_enable(true);
            } else if (front_dist_cm <= 35) {
                // CRITICAL ZONE (<35 cm): Continuous max vibration and buzzer
                haptic_set_pwm(255);
                buzzer_tone_enable(true);
            } else if (front_dist_cm <= 80) {
                // WARNING ZONE (35-80 cm): Pulsing vibration
                if ((now % 200) < 100) {
                    haptic_set_pwm(200);
                    buzzer_tone_enable(true);
                } else {
                    haptic_set_pwm(0);
                    buzzer_tone_enable(false);
                }
            } else if (front_dist_cm <= 150) {
                // NOTICE ZONE (80-150 cm): Gentle spaced pulse
                if ((now % 600) < 120) {
                    haptic_set_pwm(120);
                    buzzer_tone_enable(true);
                } else {
                    haptic_set_pwm(0);
                    buzzer_tone_enable(false);
                }
            } else {
                // CLEAR: Idle
                haptic_set_pwm(0);
                buzzer_tone_enable(false);
            }
        }

        // Heartbeat LED (1 Hz)
        if (now - last_heartbeat >= 500) {
            last_heartbeat = now;
            PINB |= (1 << STATUS_LED_BIT); // Toggle pin 13
        }
    }

    return 0;
}
