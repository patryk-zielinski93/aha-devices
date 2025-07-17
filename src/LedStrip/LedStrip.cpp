#include "LedStrip.h"

LedStrip* LedStrip::_head = nullptr;

LedStrip::LedStrip(
    ModbusRTUMaster* modbusMaster,
    HALight* haLight,
    const char* name,
    uint8_t pin,
    int ledGroupIndex,
    uint16_t eepromAddr,
    uint8_t eepromSlots,
    uint16_t stabilizationTimeMs,
    const char* icon
) : _nextInstance(nullptr),
    _modbusMaster(modbusMaster),
    _haLight(haLight),
    _pin(pin),
    _ledGroupIndex(ledGroupIndex),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _eepromAddr(eepromAddr),
    _eepromSlots(eepromSlots),
    _stabilizationTimeMs(stabilizationTimeMs)
{
    _haNameBuffer = new char[strlen(name) + 1];
    strcpy(_haNameBuffer, name);

    if (icon)
    {
        _haIconBuffer = new char[strlen(icon) + 1];
        strcpy(_haIconBuffer, icon);
    }
    _nextInstance = _head;
    _head = this;
}

// Destruktor
LedStrip::~LedStrip()
{
    delete[] _haNameBuffer;
    delete[] _haIconBuffer;
}

// --- Metody statyczne ---
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
    for (LedStrip* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->_haLight == haLight)
        {
            return current;
        }
    }
    return nullptr;
}

// --- Callbacki z Home Assistant ---
void LedStrip::_onStateCommand(bool state, HALight* sender)
{
    LedStrip* instance = _findInstance(sender);
    if (instance)
    {
        instance->setState(state);
    }
}

void LedStrip::_onBrightnessCommand(uint8_t brightness, HALight* sender)
{
    LedStrip* instance = _findInstance(sender);
    if (instance)
    {
        instance->setBrightness(brightness);
    }
}

void LedStrip::_onRGBColorCommand(HALight::RGBColor color, HALight* sender)
{
    LedStrip* instance = _findInstance(sender);
    if (instance)
    {
        instance->setRGBColor(color);
    }
}

void LedStrip::_onColorTemperatureCommand(uint16_t mireds, HALight* sender)
{
    LedStrip* instance = _findInstance(sender);
    if (instance)
    {
        instance->setColorTemperature(mireds);
    }
}

// --- Metody instancji ---
int LedStrip::getStartAddress() const
{
    return _ledGroupIndex * REGS_PER_GROUP;
}

int LedStrip::getRegisterAddress(ModbusLedGroupRegisters reg) const
{
    return getStartAddress() + reg;
}

void LedStrip::_saveStateToEeprom()
{
    if (_eepromAddr > 0)
    {
        EepromService::write(_eepromAddr, _register, _eepromSlots);
    }
}

uint16_t LedStrip::_getMireds() const
{
    uint8_t ww_val = _register.values[REG_WW];
    uint8_t cw_val = _register.values[REG_CW];
    uint32_t total = ww_val + cw_val;
    if (total == 0) return 153; // Domyślnie zimny, jeśli oba są 0
    uint8_t ww_percent = (ww_val * 100) / total;
    return map(ww_percent, 0, 100, 153, 370);
}


void LedStrip::_updateModbusRegisters(LedGroupCommand command = LedGroupCommand::CMD_IDLE)
{
    uint8_t error;

    if (_register.values[REG_ANIMATION_ID] == 0)
    {
        _register.values[REG_MODE] = 0;
    }

    if (command != LedGroupCommand::CMD_IDLE)
    {
        uint16_t regs_to_write = REGS_PER_GROUP - REG_COMMAND;
        _register.values[REG_COMMAND] = command;
        error = _modbusMaster->writeMultipleHoldingRegisters(
            _modbusId,
            getRegisterAddress(REG_COMMAND),
            &_register.values[REG_COMMAND],
            regs_to_write
        );
        // command send, reset to CMD_IDLE
        _register.values[REG_COMMAND] = LedGroupCommand::CMD_IDLE;
    }
    else
    {
        uint16_t regs_to_write = REGS_PER_GROUP - REG_MODE;
        error = _modbusMaster->writeMultipleHoldingRegisters(
            _modbusId,
            getRegisterAddress(REG_MODE),
            &_register.values[REG_MODE],
            regs_to_write
        );
    }


    // @Todo: if error retry??
    if (error)
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINT(F(")] #_updateModbusRegisters() - Błąd zapisu Modbus: "));
        DPRINTLN(error);
    }
    else
    {
        DPRINT(F("[LedStrip("));
        DPRINT(_haLight->uniqueId());
        DPRINTLN(F(")] Konfiguracja wysłana pomyślnie."));
    }
}

void LedStrip::_setup()
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);

    if (_eepromAddr > 0)
    {
        DPRINTLN(F("[LedStrip] Odczyt rejestrów z EEPROM..."));
        _register = EepromService::read(_eepromAddr, _register, _eepromSlots);

        long sum = 0;
        for (int i = 0; i < REGS_PER_GROUP; ++i)
        {
            sum += _register.values[i];
        }

        if (sum == 0)
        {
            DPRINTLN(F("[LedStrip] EEPROM pusty, ustawianie wartości domyślnych..."));
            _register.values[REG_MODE] = 0; // MODE_STATIC
            _register.values[REG_BRIGHTNESS] = 50;
            _register.values[REG_R] = 0;
            _register.values[REG_G] = 0;
            _register.values[REG_B] = 0;
            _register.values[REG_CW] = 50;
            _register.values[REG_WW] = 150;
            _register.values[REG_ANIMATION_ID] = 0;
            _register.values[REG_ANIMATION_SPEED] = 10;
            _register.values[REG_COLOR_CYCLE] = 0; // false
            _register.values[REG_COMMAND] = CMD_IDLE;
            _register.values[REG_STATE] = STATE_IDLE_OFF;
            _saveStateToEeprom();
        }
    }

    _haLight->setName(_haNameBuffer);
    if (_haIconBuffer) _haLight->setIcon(_haIconBuffer);
    _haLight->setMaxMireds(370); // ~2700K
    _haLight->setMinMireds(153); // ~6500K
    _haLight->onStateCommand(_onStateCommand);
    _haLight->onBrightnessCommand(_onBrightnessCommand);
    _haLight->onColorTemperatureCommand(_onColorTemperatureCommand);
    _haLight->onRGBColorCommand(_onRGBColorCommand);

    // Ustawienie początkowego stanu w HA na podstawie wczytanych rejestrów
    _haLight->setState(_register.values[REG_STATE] == STATE_ON);
    _haLight->setBrightness(_register.values[REG_BRIGHTNESS]);
    _haLight->setRGBColor(HALight::RGBColor(_register.values[REG_R], _register.values[REG_G], _register.values[REG_B]));
    _haLight->setColorTemperature(_getMireds());
}

void LedStrip::_loop()
{
    switch (_state)
    {
    case LedStripState::IDLE:
        break;
    case LedStripState::TURN_ON:
        if (_turnOffSequenceStartedAt != 0)
        {
            _turnOffSequenceStartedAt = 0;
            _turnOffModbusReadAt = 0;
        }

        if (_turnOnSequenceStartedAt == 0)
        {
            _updateModbusRegisters(LedGroupCommand::CMD_PREPARE_TURN_ON);
            _turnOnSequenceStartedAt = millis();
        }
        else if (millis() - _turnOnSequenceStartedAt > _stabilizationTimeMs)
        {
            _updateModbusRegisters(LedGroupCommand::CMD_TURN_ON);
            _state = LedStripState::IDLE;
            _turnOnSequenceStartedAt = 0;
        }
        break;
    case LedStripState::TURN_OFF:
        if (_turnOnSequenceStartedAt != 0)
        {
            _turnOnSequenceStartedAt = 0;
        }

        if (_turnOffSequenceStartedAt == 0)
        {
            _updateModbusRegisters(LedGroupCommand::CMD_PREPARE_TURN_OFF);
            _turnOffSequenceStartedAt = millis();
            return;
        }

        if (millis() - _turnOffModbusReadAt > 50)
        {
            uint16_t state = 0;
            auto error = _modbusMaster->readHoldingRegisters(_modbusId, getRegisterAddress(REG_STATE), &state, 1);
            _turnOffModbusReadAt = millis();
            if (error == 0 && static_cast<LedGroupState>(state) == LedGroupState::STATE_READY_TO_TURN_OFF)
            {
                _updateModbusRegisters(LedGroupCommand::CMD_TURN_OFF);
                _state = LedStripState::IDLE;
                _turnOffSequenceStartedAt = 0;
                _turnOffModbusReadAt = 0;
            }
        }

        if (millis() - _turnOffSequenceStartedAt > 3000)
        {
            // max 3 seconds for turn off animation, if timeout reached send TURN_OFF command
            DPRINTLN(F("Timeout for TURN_OFF animation reached"));
            _updateModbusRegisters(LedGroupCommand::CMD_TURN_OFF);
            _state = LedStripState::IDLE;
            _turnOffSequenceStartedAt = 0;
            _turnOffModbusReadAt = 0;
            return;
        }
        break;
    }
}

LedGroupCommand LedStrip::_getCurrentCommand()
{
    return static_cast<LedGroupCommand>(_register.values[REG_COMMAND]);
}

void LedStrip::executeCommand(LedGroupCommand command)
{
    _register.values[REG_COMMAND] = command;
}

// --- Metody publiczne API ---
void LedStrip::setState(bool state)
{
    _isTurnedOn = state;
    _haLight->setState(state, false);
    _state = state ? LedStripState::TURN_ON : LedStripState::TURN_OFF;
}

bool LedStrip::getState()
{
    return _isTurnedOn;
}

void LedStrip::setBrightness(uint8_t brightness)
{
    _register.values[REG_BRIGHTNESS] = brightness;
    _haLight->setBrightness(brightness, false);

    if (getState())
    {
        _updateModbusRegisters();
    }
    else
    {
        setState(true);
    }
    _saveStateToEeprom();
}

void LedStrip::setRGBColor(HALight::RGBColor color)
{
    _register.values[REG_R] = color.red;
    _register.values[REG_G] = color.green;
    _register.values[REG_B] = color.blue;
    _haLight->setRGBColor(color, false);

    if (getState())
    {
        _updateModbusRegisters();
    }
    else
    {
        setState(true);
    }
    _saveStateToEeprom();
}

void LedStrip::setColorTemperature(uint16_t mireds)
{
    // Konwersja mireds na wartości CW/WW (zakres 0-255)
    uint8_t ww_percent = map(mireds, 153, 370, 0, 100);
    uint8_t cw_percent = 100 - ww_percent;

    _register.values[REG_WW] = (255 * ww_percent) / 100;
    _register.values[REG_CW] = (255 * cw_percent) / 100;
    _haLight->setColorTemperature(mireds, false);

    if (getState())
    {
        _updateModbusRegisters();
    }
    else
    {
        setState(true);
    }
    _saveStateToEeprom();
}
