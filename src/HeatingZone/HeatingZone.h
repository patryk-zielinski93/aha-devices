#ifndef AHA_DEVICES_HEATINGZONE_H
#define AHA_DEVICES_HEATINGZONE_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "EepromSerivce.h"

enum HeatingCircuitCurrentState : uint8_t
{
    STATE_MODE_OFF,
    STATE_MODE_HEAT
};

class HeatingZone
{
public:
    inline static unsigned long valveStateChangeDuration = 120000; // 2 mins
    inline static uint16_t features = HAHVAC::TargetTemperatureFeature | HAHVAC::ModesFeature;

    static bool isHeatNeededOnTheFloor(uint8_t floor);
    static void setup();
    static void updateHAStates();

    HeatingZone(
        HAHVAC* haHVAC,
        const uint8_t* valvePins,
        uint8_t valveCount,
        uint8_t floor,
        uint8_t zoneNumber,
        // --- EEPROM Configuration ---
        uint16_t eepromAddrState,
        uint16_t eepromAddrTargetTemperature,
        // --- Optional HA Configuration ---
        const char* name = nullptr,
        const char* icon = nullptr
    ) : _haHVAC(haHVAC),
        _valvePins(nullptr),
        _valveCount(valveCount),
        _floor(floor),
        _zoneNumber(zoneNumber),
        _eepromAddrState(eepromAddrState),
        _eepromAddrTargetTemperature(eepromAddrTargetTemperature)
    {
        _haHVAC->setName(name == nullptr ? _haHVAC->uniqueId() : name);
        if (icon) _haHVAC->setIcon(icon);
        _haHVAC->setModes(HAHVAC::HeatMode | HAHVAC::OffMode);

        // Copy valve pins to internal array
        if (valveCount > 0 && valvePins != nullptr)
        {
            _valvePins = new uint8_t[valveCount];
            for (uint8_t i = 0; i < valveCount; i++)
            {
                _valvePins[i] = valvePins[i];
                pinMode(_valvePins[i], OUTPUT);
                digitalWrite(_valvePins[i], HIGH);
            }
        }

        _nextInstance = _head;
        _head = this;
    }


    void setCurrentTemperature(float temperature);
    float getCurrentTemperature();
    void setTargetTemperature(float temperature);
    float getTargetTemperature();
    bool isHeatNeeded() const;

private:
    HAHVAC* _haHVAC;
    uint8_t* _valvePins;
    uint8_t _valveCount;
    uint8_t _valveState;
    uint8_t _floor;
    uint8_t _zoneNumber;
    uint16_t _eepromAddrState;
    uint16_t _eepromAddrTargetTemperature;
    HAHVAC::Action _currentAction = HAHVAC::UnknownAction;
    unsigned long _valveStateChange = 0;

    float _currentTemperature = 0;
    float _targetTemperature = 23;
    HAHVAC::Mode _mode = HAHVAC::HeatMode;

    void _update();
    void _setup();
    void _setAllValves(uint8_t state);
    void _setAction(HAHVAC::Action action);
    void _setMode(HAHVAC::Mode mode);

    // --- Linked List for Instance Management ---
    HeatingZone* _nextInstance;
    static HeatingZone* _head;

    // --- Private static callback handling (no prefix) ---
    static HeatingZone* findInstance(HAHVAC* haHVAC);
    static void onTargetTemperatureCommand(HANumeric temperature, HAHVAC* sender);
    static void onModeCommand(HAHVAC::Mode mode, HAHVAC* sender);
};


#endif //AHA_DEVICES_HEATINGZONE_H
