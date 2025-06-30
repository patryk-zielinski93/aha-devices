//
// Created by Patryk Zieliński on 23/07/2024.
//

#include "MotionSensor.h"

void MotionSensor::_handleMotionDetected() {
    DPRINTLN("[MotionSensor] #" + String(_id) + " motion detected");
    if (_motionCallback != nullptr) {
        _motionCallback(this);
    }
}

void MotionSensor::setup() const {
    pinMode(_sensorPin, INPUT);
    DPRINTLN("[MotionSensor] #" + String(_id) + " initialized");
}

void MotionSensor::loop() {
    if (digitalRead(_sensorPin) == HIGH) {
        if (!_motionDetected) {
            _motionDetected = true;
            _handleMotionDetected();
        }
    } else {
        _motionDetected = false;
    }
}
