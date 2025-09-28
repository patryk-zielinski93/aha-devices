//
// Created by zielq on 29.09.2025.
//

#ifndef AHA_DEVICES_HEATINGCONTROLLER_H
#define AHA_DEVICES_HEATINGCONTROLLER_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "DS18B20/DS18B20.h"
#include "HeatingZone/HeatingZone.h"

struct PumpStateUpdate
{
    uint8_t pumpPin;
    bool state;
};

struct CircuitPump
{
    uint8_t pumpPin;
    uint8_t floor;
};

struct HeatingZoneControl
{
    DS18B20* sensorManager;
    uint8_t sensorIndex;
    HeatingZone* heatingZone;
};

class HeatingController
{
public:
    HeatingController(
        CircuitPump* pumps,
        uint8_t pumpsCount,
        HeatingZoneControl* heatingZoneControls,
        uint8_t heatingZoneControlsCount
    ) : _pumps(pumps),
        _pumpsCount(pumpsCount),
        _heatingZoneControls(heatingZoneControls),
        _heatingZoneControlsCount(heatingZoneControlsCount)
    {
    }

    float getHighestTargetTemperature();
    bool isHeatNeeded();
    void setup();
    void loop();

private :
    CircuitPump* _pumps;
    uint8_t _pumpsCount;
    HeatingZoneControl* _heatingZoneControls;
    uint8_t _heatingZoneControlsCount;
    unsigned long _updateInterval = 60000;
    unsigned long _lastUpdate = 0;

    void _updatePumps();
    void _updateTemperatures();
};


#endif //AHA_DEVICES_HEATINGCONTROLLER_H
