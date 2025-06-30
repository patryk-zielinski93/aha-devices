//
// Created by zielq on 24.11.2024.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <ArduinoHA.h>

#include "Debug.h"

class Motor {
private:
    enum MotorState {
        StateIdle,
        StateTargetingPosition
    };

    enum MotorDirection {
        DirectionNone,
        DirectionOpen,
        DirectionClose
    };

    inline static Motor** _instances = nullptr;
    inline static uint8_t _instanceIndexCounter = 0;

    static Motor* _findMotorByHACover(HACover* haCover);

    static void _onCommand(HACover::CoverCommand command, HACover* sender);

    HACover* _haCover;
    MotorDirection _direction = DirectionNone;
    MotorState _state = StateIdle;
    int _currentPositionMs = 0;
    int _targetPositionMs = 0;
    int _motorOpenPin = 0;
    int _motorClosePin = 0;
    int _fullCourseTimeMs = 0;
    unsigned long _lastUpdatedAt = 0;
    unsigned long _safetyDelayEnd = 0;

    void _motorOpen();
    void _motorClose();
    void _motorStop();
    void _stateIdle();
    void _stateTargetingPosition();
    void _startSafetyDelay();
    bool _isSafetyDelayActive();

public:
    Motor(HACover* haCover,
          const char* name,
          uint8_t motorOpenPin,
          uint8_t motorClosePin,
          int fullCourseTimeMs,
          const char* icon = nullptr) :
        _haCover(haCover),
        _motorOpenPin(motorOpenPin),
        _motorClosePin(motorClosePin),
        _fullCourseTimeMs(fullCourseTimeMs) {
        haCover->setName(name);
        if (icon != nullptr) {
            haCover->setIcon(icon);
        }

        // Reallocating to accommodate the new instance
        if (_instances != nullptr) {
            _instances = (Motor**)realloc(_instances, (_instanceIndexCounter + 1) * sizeof(Motor*));
        }
        else {
            _instances = (Motor**)malloc((_instanceIndexCounter + 1) * sizeof(Motor*));
        }

        _instances[_instanceIndexCounter] = this;
        _instanceIndexCounter++;
    }

    void setup();

    void loop();

    void open();

    void close();

    void stop();

    bool isTargeting() const;
};


#endif //MOTOR_H
