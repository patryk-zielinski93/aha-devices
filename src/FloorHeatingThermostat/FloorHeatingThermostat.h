//
// Created by zielq on 21.10.2025.
//

#ifndef AHA_DEVICES_FLOORHEATINGTHERMOSTAT_H
#define AHA_DEVICES_FLOORHEATINGTHERMOSTAT_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "EepromSerivce.h"
#include "device-types/HAHVAC.h"

class FloorHeatingThermostat
{
public:
    inline static uint16_t features = HAHVAC::TargetTemperatureFeature | HAHVAC::ModesFeature | HAHVAC::ActionFeature;
    inline static float MAX_TEMPERATURE = 25.0f;
    inline static float MIN_TEMPERATURE = 18.0f;
    inline static float TEMPERATURE_STEP = 0.5f;
    inline static float DEFAULT_TEMPERATURE = 21.5f;
    static void setup();

    FloorHeatingThermostat(
        HAHVAC* haHVACp,
        // --- EEPROM Configuration ---
        uint16_t eepromAddrState,
        uint16_t eepromAddrTargetTemperature,
        // --- Optional HA Configuration ---
        float minTemperature = MIN_TEMPERATURE,
        float maxTemperature = MAX_TEMPERATURE,
        float temperatureStep = TEMPERATURE_STEP,
        const char* name = nullptr,
        const char* icon = nullptr
    ) : haHVAC(haHVACp),
        _eepromAddrState(eepromAddrState),
        _eepromAddrTargetTemperature(eepromAddrTargetTemperature)
    {
        haHVAC->setName(name == nullptr ? haHVAC->uniqueId() : name);
        if (icon) haHVAC->setIcon(icon);
        haHVAC->setModes(HAHVAC::AutoMode | HAHVAC::OffMode);
        haHVAC->setMinTemp(minTemperature);
        haHVAC->setMaxTemp(maxTemperature);
        haHVAC->setTempStep(temperatureStep);
        haHVAC->setAction(HAHVAC::Action::IdleAction);
        haHVAC->setCurrentCurrentTemperature(0);

        _nextInstance = _head;
        _head = this;
    }

    HAHVAC* haHVAC;

    bool isHeatNeeded();

private:
    uint16_t _eepromAddrState;
    uint16_t _eepromAddrTargetTemperature;

    void _setup();

    // --- Linked List for Instance Management ---
    FloorHeatingThermostat* _nextInstance;
    static FloorHeatingThermostat* _head;

    // --- Private static callback handling (no prefix) ---
    static FloorHeatingThermostat* findInstance(HAHVAC* haHVAC);
    static void onTargetTemperatureCommand(HANumeric temperature, HAHVAC* sender);
    static void onModeCommand(HAHVAC::Mode mode, HAHVAC* sender);
};


#endif //AHA_DEVICES_FLOORHEATINGTHERMOSTAT_H
