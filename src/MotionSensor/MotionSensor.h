//
// Created by Patryk Zieliński on 23/07/2024.
//

#ifndef AHA_DEVICES_MOTIONSENSOR_H
#define AHA_DEVICES_MOTIONSENSOR_H

#include <Arduino.h>

#include "Debug.h"

#define MOTION_SENSOR_MOTION_CALLBACK(name) void (*name)(MotionSensor *sender)

class MotionSensor {
private:
    uint8_t _id;
    uint8_t _sensorPin;
    bool _motionDetected = false;

    MOTION_SENSOR_MOTION_CALLBACK(_motionCallback) = nullptr;

    void _handleMotionDetected();

public:
    explicit MotionSensor(
            uint8_t id,
            uint8_t sensorPin
    ) : _id(id),
        _sensorPin(sensorPin) {}

    inline void onMotionCallback(MOTION_SENSOR_MOTION_CALLBACK(callback)) {
        _motionCallback = callback;
    }

    inline uint8_t getId() const {
        return _id;
    }

    void setup() const;

    void loop();
};


#endif //AHA_DEVICES_MOTIONSENSOR_H
