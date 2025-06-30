//
// Created by zielq on 24.11.2024.
//

#include "Motor.h"

Motor* Motor::_findMotorByHACover(HACover* haCover) {
    DPRINTLN("[Motor] Looking for HACover in _instances:");
    for (uint8_t i = 0; i < _instanceIndexCounter; i++) {
        if (_instances[i]->_haCover == haCover) {
            DPRINTLN("[Motor] Match found for HACover at instance: " + String((uintptr_t) _instances[i]));
            return _instances[i];
        }
    }
    return nullptr;
}

void Motor::_onCommand(HACover::CoverCommand command, HACover* sender) {
    DPRINTLN("[Cover] #_onCommand(" + String(command) + ")");
    Motor* motor = _findMotorByHACover(sender);

    if (motor == nullptr) {
        return;
    }

    switch (command) {
    case HACover::CoverCommand::CommandStop:
        motor->stop();
        break;
    case HACover::CoverCommand::CommandOpen:
        motor->open();
        break;
    case HACover::CoverCommand::CommandClose:
        motor->close();
        break;
    }
}

void Motor::setup() {
    pinMode(_motorOpenPin, OUTPUT);
    pinMode(_motorClosePin, OUTPUT);
    _motorStop();

    _haCover->onCommand(_onCommand);
    _haCover->setCurrentState(HACover::CoverState::StateStopped);

    _haCover->setOptimistic(true);
    _haCover->setRetain(true);
}

void Motor::loop() {
    switch (_state) {
    case StateIdle:
        _stateIdle();
        break;
    case StateTargetingPosition:
        _stateTargetingPosition();
        break;
    }
}

void Motor::_startSafetyDelay() {
    DPRINTLN("[Motor] #_startSafetyDelay()");
    _safetyDelayEnd = millis() + 250;
}

void Motor::_motorOpen() {
    DPRINTLN("[Motor] #_motorOpen()");
    digitalWrite(_motorClosePin, LOW);
    digitalWrite(_motorOpenPin, HIGH);
    _direction = DirectionOpen;
}

void Motor::_motorClose() {
    DPRINTLN("[Motor] #_motorClose()");
    digitalWrite(_motorOpenPin, LOW);
    digitalWrite(_motorClosePin, HIGH);
    _direction = DirectionClose;
}

void Motor::_motorStop() {
    DPRINTLN("[Motor] #_motorStop()");
    digitalWrite(_motorOpenPin, LOW);
    digitalWrite(_motorClosePin, LOW);
    _direction = DirectionNone;
    _startSafetyDelay();
    _haCover->setState(HACover::CoverState::StateStopped);
}

void Motor::_stateIdle() {
    if (_currentPositionMs != _targetPositionMs) {
        _state = StateTargetingPosition;
        DPRINTLN("[Cover] #_stateIdle() -> StateTargetingPosition");
    }
}

void Motor::_stateTargetingPosition() {
    if (_isSafetyDelayActive()) {
        _lastUpdatedAt = millis();
        return;
    }

    if (DirectionNone == _direction) {
        if (_currentPositionMs < _targetPositionMs) {
            _motorOpen();
        }
        else if (_currentPositionMs > _targetPositionMs) {
            _motorClose();
        }
        else if (_currentPositionMs == _targetPositionMs) {
            _state = StateIdle;
            DPRINTLN("[Motor] #_stateTargetingPosition() -> _state=StateIdle");
        }

        _lastUpdatedAt = millis();
        return;
    }

    if (DirectionOpen == _direction) {
        uint32_t change = millis() - _lastUpdatedAt;
        _lastUpdatedAt = millis();
        _currentPositionMs += change;

        if (_currentPositionMs >= _targetPositionMs) {
            if (_currentPositionMs >= _fullCourseTimeMs && _currentPositionMs - _fullCourseTimeMs < 500) {
                // 0,5 sec calibration on each full course
                DPRINTLN("[Motor] #_stateTargeting() -> calibration active");
                return;
            }

            if (_currentPositionMs > _fullCourseTimeMs) {
                _currentPositionMs = _fullCourseTimeMs;
            }

            _targetPositionMs = _currentPositionMs;
            _motorStop();
            DPRINTLN("[Motor] #_stateTargeting() direction open -> _motorStop()");
            _state = StateIdle;
        }
    }

    if (DirectionClose == _direction) {
        uint32_t change = millis() - _lastUpdatedAt;
        _lastUpdatedAt = millis();
        _currentPositionMs -= change;

        if (_currentPositionMs <= _targetPositionMs) {
            if (_currentPositionMs <= 0 && _currentPositionMs > -500) {
                // 0,5 sec calibration on each full course
                DPRINTLN("[Motor] #_stateTargeting() -> calibration active");
                return;
            }

            if (_currentPositionMs < 0) {
                _currentPositionMs = 0;
            }

            _targetPositionMs = _currentPositionMs;
            _motorStop();
            DPRINTLN("[Motor] #_stateTargeting() direction close -> _motorStop()");
            _state = StateIdle;
        }
    }
}

bool Motor::_isSafetyDelayActive() {
    return millis() < _safetyDelayEnd;
}

void Motor::open() {
    DPRINTLN("[Motor] #open()");
    _targetPositionMs = _fullCourseTimeMs;
}

void Motor::close() {
    DPRINTLN("[Motor] #close()");
    _targetPositionMs = 0;
}

void Motor::stop() {
    DPRINTLN("[Motor] #stop()");
    _targetPositionMs = _currentPositionMs;
    _motorStop();
    _state = StateIdle;
}

bool Motor::isTargeting() const {
    return _state == StateTargetingPosition;
}
