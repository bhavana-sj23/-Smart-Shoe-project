/**
 * @file filter.h
 * @brief Outlier rejection and smoothing filters (Median + Exponential Moving Average)
 *        to reject gait vibration and ground reflection noise in ultrasonic telemetry.
 */

#ifndef FILTER_H
#define FILTER_H

#include <Arduino.h>

template <size_t N>
class MedianFilter {
private:
    float buffer[N];
    size_t index;
    size_t count;

public:
    MedianFilter() : index(0), count(0) {
        for (size_t i = 0; i < N; ++i) {
            buffer[i] = 0.0f;
        }
    }

    void reset(float initialVal = 0.0f) {
        index = 0;
        count = 0;
        for (size_t i = 0; i < N; ++i) {
            buffer[i] = initialVal;
        }
    }

    float update(float newSample) {
        buffer[index] = newSample;
        index = (index + 1) % N;
        if (count < N) count++;

        // Insertion sort on a local copy to compute median
        float sorted[N];
        for (size_t i = 0; i < count; ++i) {
            sorted[i] = buffer[i];
        }

        for (size_t i = 1; i < count; ++i) {
            float key = sorted[i];
            int j = i - 1;
            while (j >= 0 && sorted[j] > key) {
                sorted[j + 1] = sorted[j];
                j--;
            }
            sorted[j + 1] = key;
        }

        return sorted[count / 2];
    }
};

class ExponentialFilter {
private:
    float alpha;
    float currentVal;
    bool initialized;

public:
    explicit ExponentialFilter(float smoothingFactor = 0.35f)
        : alpha(smoothingFactor), currentVal(0.0f), initialized(false) {}

    void reset(float val = 0.0f) {
        currentVal = val;
        initialized = false;
    }

    float update(float newSample) {
        if (!initialized) {
            currentVal = newSample;
            initialized = true;
            return currentVal;
        }
        currentVal = (alpha * newSample) + ((1.0f - alpha) * currentVal);
        return currentVal;
    }

    float getValue() const {
        return currentVal;
    }
};

#endif // FILTER_H
