//
// Created by zielq on 25.10.2025.
//

#ifndef AHA_DEVICES_HEATINGZONE_H
#define AHA_DEVICES_HEATINGZONE_H

#include <Arduino.h>
#include <device-types/HAHVAC.h>

#include "Valve/ValveState.h"


struct HeatingZone
{
    // Thermostat data
    HAHVAC* haHVAC;
    uint16_t eepromAddrState;
    uint16_t eepromAddrTargetTemperature;

    // Valve data
    uint8_t valvePin;
    ValveState valveCurrentState;
    unsigned long valveStateChangeFinishedAt; // 0 = stan stabilny

    // HeatingZone data
    uint8_t tempSensorIndex;
    uint8_t floor;
};


#endif //AHA_DEVICES_HEATINGZONE_H
