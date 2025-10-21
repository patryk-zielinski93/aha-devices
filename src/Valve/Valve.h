//
// Created by zielq on 21.10.2025.
//

#ifndef AHA_DEVICES_VALVE_H
#define AHA_DEVICES_VALVE_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "EepromSerivce.h"

enum ValveState : uint8_t
{
    VALVE_STATE_CLOSED = 0,
    VALVE_STATE_OPEN = 1,
    VALVE_STATE_CLOSING = 2,
    VALVE_STATE_OPENING = 3
};

class Valve
{
public:
    Valve(
        uint8_t valvePin,
        unsigned long stateChangeDurationMs = 120000,
        ValveState currentState = ValveState::VALVE_STATE_OPEN
    ) : _stateChangeDurationMs(stateChangeDurationMs),
        _currentState(currentState),
        _valvePin(valvePin)
    {
        pinMode(_valvePin, OUTPUT);
        digitalWrite(_valvePin, LOW);

        // Add to linked list
        _nextInstance = _head;
        _head = this;
    }

    static void loop();

    void open();
    void close();
    ValveState getState();

private:
    unsigned long _stateChangeDurationMs;
    ValveState _currentState;
    unsigned long _stateChangeFinishedAt = 0;
    uint8_t _valvePin;

    void _loop();

    // --- Linked List for Instance Management ---
    Valve* _nextInstance;
    static Valve* _head;
};


#endif //AHA_DEVICES_VALVE_H
