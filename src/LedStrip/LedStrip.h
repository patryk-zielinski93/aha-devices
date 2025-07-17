#ifndef AHA_DEVICES_LEDSTRIP_H
#define AHA_DEVICES_LEDSTRIP_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include <ModbusRTUMaster.h>
#include "Debug.h"
#include "EepromSerivce.h"
#include "LedGroupModbusRegisters.h"

struct LedStripRegisterSet
{
    uint16_t values[REGS_PER_GROUP];
};


class LedStrip
{
public:
    LedStrip(
        ModbusRTUMaster* modbusMaster,
        HALight* haLight,
        const char* name,
        uint8_t pin,
        int ledGroupIndex,
        uint16_t eepromAddr = 0,
        uint8_t eepromSlots = 10,
        uint16_t stabilizationTimeMs = 200,
        const char* icon = nullptr
    );

    virtual ~LedStrip();

    // Statyczne metody setup() i loop() dla wszystkich instancji
    static void setup();
    static void loop();

    // Publiczne API
    void setState(bool state);
    bool getState();
    void setBrightness(uint8_t brightness);
    void setRGBColor(HALight::RGBColor color);
    void setColorTemperature(uint16_t mireds);
    int getStartAddress() const;

private:
    enum LedStripState
    {
        IDLE = 0,
        TURN_ON = 1,
        TURN_OFF = 2
    };

    LedStrip* _nextInstance;
    ModbusRTUMaster* _modbusMaster;
    HALight* _haLight;
    uint8_t _pin;
    int _ledGroupIndex;
    char* _haNameBuffer;
    char* _haIconBuffer;
    uint8_t _modbusId = 1;
    LedStripState _state = IDLE;
    bool _isTurnedOn = false;

    // EEPROM
    uint16_t _eepromAddr;
    uint8_t _eepromSlots;
    LedStripRegisterSet _register{};

    uint16_t _stabilizationTimeMs;
    unsigned long _turnOnSequenceStartedAt = 0;
    unsigned long _turnOffSequenceStartedAt = 0;
    unsigned long _turnOffModbusReadAt = 0;

    void _setup();
    void _loop();
    LedGroupCommand _getCurrentCommand();
    void executeCommand(LedGroupCommand command);
    void _saveStateToEeprom();
    uint16_t _getMireds() const;
    void _updateModbusRegisters(LedGroupCommand command);
    int getRegisterAddress(ModbusLedGroupRegisters reg) const;

    // -- Home assistant --
    static LedStrip* _head;
    static LedStrip* _findInstance(HALight* haLight);
    static void _onStateCommand(bool state, HALight* sender);
    static void _onBrightnessCommand(uint8_t brightness, HALight* sender);
    static void _onColorTemperatureCommand(uint16_t mireds, HALight* sender);
    static void _onRGBColorCommand(HALight::RGBColor color, HALight* sender);
};

#endif //AHA_DEVICES_LEDSTRIP_H
