//
// Created by zielq on 29.09.2025.
//

#include "HeatingController.h"

float HeatingController::getHighestTargetTemperature()
{
    float highest = 18;
    for (uint8_t i = 0; i < _heatingZoneControlsCount; i++)
    {
        float temp = _heatingZoneControls[i].heatingZone->getTargetTemperature();
        if (temp > highest)
        {
            highest = temp;
        }
    }

    return highest;
}

bool HeatingController::isHeatNeeded()
{
    for (uint8_t i = 0; i < _pumpsCount; i++)
    {
        if (HeatingZone::isHeatNeededOnTheFloor(_pumps[i].floor))
        {
            return true;
        }
    }

    return false;
}

void HeatingController::setup()
{
    for (uint8_t i = 0; i < _pumpsCount; ++i)
    {
        pinMode(_pumps[i].pumpPin, OUTPUT);
        digitalWrite(_pumps[i].pumpPin, LOW);
        DPRINT(F("[HeatingController] Initialized pump on pin: "));
        DPRINTLN(_pumps[i].pumpPin);
    }

    HeatingZone::setup();
}

void HeatingController::loop()
{
    for (uint8_t i = 0; i < _heatingZoneControlsCount; ++i)
    {
        if (_heatingZoneControls[i].sensorManager != nullptr)
        {
            _heatingZoneControls[i].sensorManager->loop();
        }
    }

    uint32_t now = millis();
    if (now - _lastUpdate >= _updateInterval)
    {
        _lastUpdate = now;

        _updateTemperatures();
        _updatePumps();
    }
}

void HeatingController::_updatePumps()
{
    for (size_t i = 0; i < _pumpsCount; ++i)
    {
        uint8_t floor = _pumps[i].floor;
        bool isNeeded = HeatingZone::isHeatNeededOnTheFloor(floor);

        digitalWrite(_pumps[i].pumpPin, isNeeded ? HIGH : LOW);
    }
}

void HeatingController::_updateTemperatures()
{
    for (uint8_t i = 0; i < _heatingZoneControlsCount; ++i)
    {
        if (_heatingZoneControls[i].sensorManager != nullptr &&
            _heatingZoneControls[i].heatingZone != nullptr)
        {
            float temp = _heatingZoneControls[i].sensorManager->getTemperature(
                _heatingZoneControls[i].sensorIndex
            );
            DPRINT(F("[HeatingController] Current temperature: "));
            DPRINTLN(temp);
            if (temp == -127.0f)
            {
                continue;
            }
            _heatingZoneControls[i].heatingZone->setCurrentTemperature(temp);
        }
    }
}
