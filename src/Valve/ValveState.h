//
// Created by zielq on 25.10.2025.
//

#ifndef AHA_DEVICES_VALVESTATE_H
#define AHA_DEVICES_VALVESTATE_H
#include <Arduino.h>

enum ValveState : uint8_t
{
    VALVE_STATE_CLOSED = 0,
    VALVE_STATE_OPEN = 1,
    VALVE_STATE_CLOSING = 2,
    VALVE_STATE_OPENING = 3
};

#endif //AHA_DEVICES_VALVESTATE_H
