//
// Created by zielq on 25.03.2025.
//

#include "LedStrip.h"
#include "EepromSerivce.h"

LedStrip* LedStrip::_head = nullptr;

LedStrip::LedStrip(
    ModbusRTUMaster* modbusMaster,
    HALight* haLight,
    const __FlashStringHelper* name,
    uint8_t pin,
    int ledStripIndexInController,
    uint16_t eepromAddr,
    uint8_t eepromSlots,
    const __FlashStringHelper* icon
) : _nextInstance(nullptr),
    _modbusMaster(modbusMaster),
    _haLight(haLight),
    _pin(pin),
    _ledStripIndexInController(ledStripIndexInController),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _eepromAddr(eepromAddr),
    _eepromSlots(eepromSlots)
{
    _initialize(name, icon);
}

LedStrip::LedStrip(
    ModbusRTUMaster* modbusMaster,
    HALight* haLight,
    const char* name,
    uint8_t pin,
    int ledStripIndexInController,
    uint16_t eepromAddr,
    uint8_t eepromSlots,
    const char* icon
) : _nextInstance(nullptr),
    _modbusMaster(modbusMaster),
    _haLight(haLight),
    _pin(pin),
    _ledStripIndexInController(ledStripIndexInController),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _eepromAddr(eepromAddr),
    _eepromSlots(eepromSlots)
{
    _initialize(name, icon);
}

LedStrip::~LedStrip()
{
    delete[] _haNameBuffer;
    delete[] _haIconBuffer;
}

void LedStrip::_initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon)
{
    if (name)
    {
        size_t nameLen = strlen_P(reinterpret_cast<const char*>(name));
        _haNameBuffer = new char[nameLen + 1];
        strcpy_P(_haNameBuffer, reinterpret_cast<const char*>(name));
    }

    if (icon)
    {
        size_t iconLen = strlen_P(reinterpret_cast<const char*>(icon));
        _haIconBuffer = new char[iconLen + 1];
        strcpy_P(_haIconBuffer, reinterpret_cast<const char*>(icon));
    }

    _initialize(_haNameBuffer, _haIconBuffer);
}

void LedStrip::_initialize(const char* name, const char* icon)
{
    if (name && !_haNameBuffer)
    {
        _haNameBuffer = new char[strlen(name) + 1];
        strcpy(_haNameBuffer, name);
    }
    if (icon && !_haIconBuffer)
    {
        _haIconBuffer = new char[strlen(icon) + 1];
        strcpy(_haIconBuffer, icon);
    }

    _haLight->setName(_haNameBuffer);
    if (_haIconBuffer)
    {
        _haLight->setIcon(_haIconBuffer);
    }

    _nextInstance = _head;
    _head = this;
}


void LedStrip::_setup()
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);

    if (_eepromAddr > 0)
    {
        DPRINTLN(F("[LedStrip] Reading register set from EEPROM..."));
        // Odczytujemy CAŁĄ strukturę. Jeśli w EEPROM nie ma poprawnych danych,
        // użyta zostanie domyślna wartość _registers (wypełniona zerami).
        _register = EepromService::read(_eepromAddr, _register, _eepromSlots);
    }


    _haLight->setMaxMireds(370);
    _haLight->onStateCommand(_onStateCommand);
    _haLight->onBrightnessCommand(_onBrightnessCommand);
    _haLight->onColorTemperatureCommand(_onColorTemperatureCommand);
    _haLight->onRGBColorCommand(_onRGBColorCommand);
    _haLight->setBrightness(_register.values[REG_BRIGHTNESS]);
    _haLight->setRGBColor(HALight::RGBColor(_register.values[REG_R], _register.values[REG_G], _register.values[REG_B]));
    _haLight->setColorTemperature(_getMireds());
}

void LedStrip::_saveStateToEeprom()
{
    if (_eepromAddr > 0)
    {
        EepromService::write(_eepromAddr, _register, _eepromSlots);
    }
}

void LedStrip::setState(bool state)
{
    if (!_initialized && _register.values[REG_STATE_COMMAND])
    {
        _initialized = true;
        _sendCurrentLedStripDataToController();
    }

    const uint16_t command = state ? CMD_PREPARE_TURN_ON : CMD_PREPARE_TURN_OFF;
    const auto error = _modbusMaster->writeSingleHoldingRegister(
        _modbusId,
        getRegisterAddress(REG_STATE_COMMAND),
        command
    );

    if (error)
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINT(F(")] #setState() - Modbus write error: "));
        DPRINTLN(error);
        return;
    }

    _register.values[REG_STATE_COMMAND] = command;
    _saveStateToEeprom();
}

void LedStrip::setBrightness(uint8_t brightness)
{
    if (_register.values[REG_BRIGHTNESS] == brightness)
    {
        return;
    }

    const auto error = _modbusMaster->writeSingleHoldingRegister(_modbusId, getRegisterAddress(REG_BRIGHTNESS),
                                                                 brightness);
    if (error)
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINT(F(")] #setBrightness() - Modbus write error: "));
        DPRINTLN(error);
        return;
    }

    _register.values[REG_BRIGHTNESS] = brightness;
    _haLight->setBrightness(brightness);
    _saveStateToEeprom();
}

void LedStrip::setColorTemperature(uint16_t colorTemperature)
{
    // Calculate cw and ww values from mireds
    float kelvin = _miredsToKelvin(colorTemperature);
    float minKelvin = 2700;
    float maxKelvin = 6535;
    float ratio = (kelvin - minKelvin) / (maxKelvin - minKelvin);

    uint16_t cw = round(255 * ratio);
    uint16_t ww = 255 - cw;

    if (cw == _register.values[REG_CW] && ww == _register.values[REG_WW])
    {
        return;
    }

    uint16_t newValues[5] = {0, 0, 0, cw, ww};
    const auto error = _modbusMaster->writeMultipleHoldingRegisters(_modbusId, getRegisterAddress(REG_R), newValues, 5);
    if (error)
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINT(F(")] #setColorTemperature() - Modbus write error: "));
        DPRINTLN(error);
        return;
    }

    _register.values[REG_R] = 0;
    _register.values[REG_G] = 0;
    _register.values[REG_B] = 0;
    _register.values[REG_CW] = cw;
    _register.values[REG_WW] = ww;

    _haLight->setColorTemperature(colorTemperature);
    _saveStateToEeprom();
}

void LedStrip::setRGBColor(uint8_t r, uint8_t g, uint8_t b)
{
    if (_register.values[REG_R] == r && _register.values[REG_G] == g && _register.values[REG_B] == b)
    {
        return;
    }

    uint16_t newValues[5] = {r, g, b, 0, 0};
    const auto error = _modbusMaster->writeMultipleHoldingRegisters(_modbusId, getRegisterAddress(REG_R), newValues, 5);
    if (error)
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINT(F(")] #setRGBColor() - Modbus write error: "));
        DPRINTLN(error);
        return;
    }

    _register.values[REG_R] = r;
    _register.values[REG_G] = g;
    _register.values[REG_B] = b;
    _register.values[REG_CW] = 0;
    _register.values[REG_WW] = 0;

    _haLight->setRGBColor(HALight::RGBColor(r, g, b));
    _saveStateToEeprom();
}

void LedStrip::_sendCurrentLedStripDataToController()
{
    DPRINT(F("[LedStrip("));
    DPRINT(_haLight->uniqueId());
    DPRINTLN(F(")] #_sendCurrentLedStripDataToController()"));

    const auto error = _modbusMaster->writeMultipleHoldingRegisters(
        _modbusId,
        getStartAddress(),
        _register.values,
        REGS_PER_GROUP
    );

    if (error)
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINT(F(")] #_sendCurrentLedStripDataToController() - error: "));
        DPRINTLN(error);
        return; // Don't save to EEPROM on error
    }

    _saveStateToEeprom();
}

int LedStrip::_getMireds() const
{
    float minKelvin = 2700;
    float maxKelvin = 6500;
    float kelvin = minKelvin + ((maxKelvin - minKelvin) * (_register.values[REG_CW] / 255.0));
    int mireds = round(1000000.0 / kelvin);
    return mireds;
}

int LedStrip::getStartAddress() const
{
    return _ledStripIndexInController * REGS_PER_GROUP;
}

int LedStrip::getRegisterAddress(const LedStripRegister reg) const
{
    return getStartAddress() + reg;
}

void LedStrip::_loop()
{
    // Non-blocking state machine implementation using millis()
    const uint16_t currentCommand = _register.values[REG_STATE_COMMAND];
    uint8_t error = 0;

    // --- Power ON Sequence ---
    if (currentCommand == CMD_PREPARE_TURN_ON)
    {
        // Check slave status to see if it's ready for power
        error = _modbusMaster->readHoldingRegisters(
            _modbusId,
            getRegisterAddress(REG_STATE_STATUS),
            &_register.values[REG_STATE_STATUS],
            1
        );

        if (error)
        {
            DPRINT(F("[LedStrip("));
            DPRINT(_haLight->uniqueId());
            DPRINT(F(")] #_loop() - Modbus read error while preparing to turn ON: "));
            DPRINTLN(error);
            return;
        }

        // Check if slave is ready for power relay activation
        if (_register.values[REG_STATE_STATUS] == STATE_READY_TO_TURN_ON)
        {
            if (!_isPowerStabilizing)
            {
                // Turn on power relay and start stabilization timer
                digitalWrite(_pin, HIGH);
                _powerStabilizationStartTime = millis();
                _isPowerStabilizing = true;

                DPRINT(F("[LedStrip("));
                DPRINT(_haLight->uniqueId());
                DPRINTLN(F(")] Power relay ON, stabilizing..."));
            }
            else
            {
                // Check if stabilization time has passed (200ms)
                if (millis() - _powerStabilizationStartTime >= 200)
                {
                    // Inform slave that power is stable and ready
                    error = _modbusMaster->writeSingleHoldingRegister(_modbusId, getRegisterAddress(REG_STATE_COMMAND),
                                                                      CMD_TURN_ON);
                    if (error)
                    {
                        DPRINT(F("[LedStrip("));
                        DPRINT(_haLight->uniqueId());
                        DPRINT(F(")] #_loop() - Modbus write error during turn ON: "));
                        DPRINTLN(error);
                        // Don't update local state, retry in next loop cycle
                    }
                    else
                    {
                        _register.values[REG_STATE_COMMAND] = CMD_TURN_ON;
                        _isPowerStabilizing = false;

                        DPRINT(F("[LedStrip("));
                        DPRINT(_haLight->uniqueId());
                        DPRINTLN(F(")] Turn ON sequence completed"));
                    }
                }
            }
        }
    }
    // --- Power OFF Sequence ---
    else if (currentCommand == CMD_PREPARE_TURN_OFF)
    {
        // Reset stabilization flag during power off
        _isPowerStabilizing = false;

        // Check slave status to see if it's ready for power cut
        error = _modbusMaster->readHoldingRegisters(
            _modbusId,
            getRegisterAddress(REG_STATE_STATUS),
            &_register.values[REG_STATE_STATUS],
            1
        );

        if (error)
        {
            DPRINT(F("[LedStrip("));
            DPRINT(_haLight->uniqueId());
            DPRINT(F(")] #_loop() - Modbus read error while preparing to turn OFF: "));
            DPRINTLN(error);
            return;
        }

        // Check if slave has finished power-down animation
        if (_register.values[REG_STATE_STATUS] == STATE_READY_TO_TURN_OFF)
        {
            // Turn off power relay
            digitalWrite(_pin, LOW);

            // Inform slave that power is off and it should enter idle state
            error = _modbusMaster->writeSingleHoldingRegister(_modbusId, getRegisterAddress(REG_STATE_COMMAND),
                                                              CMD_TURN_OFF_IDLE);
            if (error)
            {
                DPRINT(F("[LedStrip("));
                DPRINT(_haLight->uniqueId());
                DPRINT(F(")] #_loop() - Modbus write error during turn OFF: "));
                DPRINTLN(error);
                // Don't update local state, retry in next loop cycle
            }
            else
            {
                _register.values[REG_STATE_COMMAND] = CMD_TURN_OFF_IDLE;

                DPRINT(F("[LedStrip("));
                DPRINT(_haLight->uniqueId());
                DPRINTLN(F(")] Turn OFF sequence completed"));
            }
        }
    }
    else
    {
        // Reset stabilization flag in idle states
        _isPowerStabilizing = false;
    }
}

void LedStrip::setup()
{
    for (LedStrip* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_setup();
    }
}

void LedStrip::loop()
{
    for (LedStrip* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_loop();
    }
}

LedStrip* LedStrip::_findInstance(HALight* haLight)
{
    DPRINTLN(F("[LedStrip] Looking for HALight..."));
    for (LedStrip* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->_haLight == haLight)
        {
            DPRINT(F("[LedStrip] Match found for HALight with name: "));
            DPRINTLN(current->_haLight->getName());
            return current;
        }
    }
    return nullptr;
}

void LedStrip::_onStateCommand(bool state, HALight* sender)
{
    DPRINT(F("[LedStrip] #_onStateCommand("));
    DPRINT(state);
    DPRINTLN(F(")"));
    if (LedStrip* ledStrip = _findInstance(sender); ledStrip != nullptr)
    {
        ledStrip->setState(state);
    }
}

void LedStrip::_onBrightnessCommand(uint8_t brightness, HALight* sender)
{
    DPRINT(F("[LedStrip] #_onBrightnessCommand("));
    DPRINT(brightness);
    DPRINTLN(F(")"));
    if (LedStrip* ledStrip = _findInstance(sender); ledStrip != nullptr)
    {
        ledStrip->setBrightness(brightness);
    }
}

void LedStrip::_onColorTemperatureCommand(uint16_t colorTemperature, HALight* sender)
{
    DPRINT(F("[LedStrip] #_onColorTemperatureCommand("));
    DPRINT(colorTemperature);
    DPRINTLN(F(")"));
    if (LedStrip* ledStrip = _findInstance(sender); ledStrip != nullptr)
    {
        ledStrip->setColorTemperature(colorTemperature);
    }
}

void LedStrip::_onRGBColorCommand(HALight::RGBColor color, HALight* sender)
{
    DPRINT(F("[LedStrip] #_onRGBCommand("));
    DPRINT(color.red);
    DPRINT(F(", "));
    DPRINT(color.green);
    DPRINT(F(", "));
    DPRINT(color.blue);
    DPRINTLN(F(")"));
    if (LedStrip* ledStrip = _findInstance(sender); ledStrip != nullptr)
    {
        ledStrip->setRGBColor(color.red, color.green, color.blue);
    }
}

float LedStrip::_miredsToKelvin(int mireds)
{
    return 1000000.0 / mireds;
}

int LedStrip::_kelvinToMireds(float kelvin)
{
    return round(1000000.0 / kelvin);
}
