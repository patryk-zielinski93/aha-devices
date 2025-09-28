#include "HeatingZone.h"

HeatingZone* HeatingZone::_head = nullptr;

bool HeatingZone::isHeatNeededOnTheFloor(uint8_t floor)
{
    for (HeatingZone* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->_floor == floor && current->isHeatNeeded())
        {
            return true;
        }
    }
    return false;
}

void HeatingZone::setup()
{
    for (HeatingZone* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_setup();
    }
    DPRINTLN(F("[HeatingZone] All zones initialized."));
}


void HeatingZone::setCurrentTemperature(float temperature)
{
    if (_currentTemperature == temperature)
    {
        return;
    }
    _currentTemperature = temperature;
    _haHVAC->setCurrentTemperature(temperature);
    _update();
    DPRINT(F("[HeatingZone] Temperature updated (C): "));
    DPRINTLN(temperature);
}

float HeatingZone::getCurrentTemperature()
{
    return _currentTemperature;
}

void HeatingZone::setTargetTemperature(const float temperature)
{
    if (_targetTemperature == temperature)
    {
        return;
    }
    _targetTemperature = temperature;
    _haHVAC->setTargetTemperature(temperature);
    EepromService::write<float>(_eepromAddrTargetTemperature, _targetTemperature);
    _update();
    DPRINT(F("[HeatingZone] Target temperature updated (C): "));
    DPRINTLN(temperature);
}

float HeatingZone::getTargetTemperature()
{
    return _targetTemperature;
}

bool HeatingZone::isHeatNeeded() const
{
    return _haHVAC->getCurrentAction() == HAHVAC::HeatingAction;
}

void HeatingZone::_update() const
{
    if (_valvePins == nullptr || _valveCount == 0)
    {
        return;
    }

    if (_currentTemperature == 0)
    {
        // not initialized yet
        return;
    }

    if (!_poweredOn)
    {
        if (_haHVAC->getCurrentAction() == HAHVAC::UnknownAction)
        {
            return;
        }
        _setAllValves(LOW);
        _haHVAC->setAction(HAHVAC::UnknownAction);
        EepromService::write<uint8_t>(_eepromAddrState, STATE_POWERED_OFF);
        DPRINTLN(F("[HeatingZone] Powered OFF"));
        return;
    }

    if (_mode == HAHVAC::OffMode)
    {
        if (_haHVAC->getCurrentAction() == HAHVAC::OffAction)
        {
            return;
        }
        _setAllValves(HIGH);
        _haHVAC->setAction(HAHVAC::OffAction);
        EepromService::write<uint8_t>(_eepromAddrState, STATE_MODE_OFF);
        DPRINTLN(F("[HeatingZone] OffMode activated - closing valves"));
        return;
    }

    if (_mode == HAHVAC::HeatMode)
    {
        // Add hysteresis to prevent frequent switching
        const float hysteresis = 0.25f; // 0.25°C hysteresis

        // Current action from HA
        HAHVAC::Action currentAction = _haHVAC->getCurrentAction();

        // Determine if heating is needed based on hysteresis
        bool shouldHeat = false;
        bool shouldStop = false;

        if (currentAction == HAHVAC::HeatingAction)
        {
            // Currently heating - stop only if target is reached plus hysteresis
            shouldStop = _currentTemperature >= (_targetTemperature + hysteresis);
            shouldHeat = !shouldStop;
        }
        else
        {
            // Not heating - start heating if below target minus hysteresis
            shouldHeat = _currentTemperature <= (_targetTemperature - hysteresis);
            shouldStop = !shouldHeat;
        }

        if (shouldHeat && currentAction != HAHVAC::HeatingAction)
        {
            _setAllValves(LOW);
            _haHVAC->setAction(HAHVAC::HeatingAction);
            EepromService::write<uint8_t>(_eepromAddrState, STATE_TARGETING_TEMPERATURE);
            DPRINTLN(F("[HeatingZone] HeatMode (HeatingAction) - opening valves"));
        }
        else if (shouldStop && currentAction != HAHVAC::IdleAction)
        {
            _setAllValves(HIGH);
            _haHVAC->setAction(HAHVAC::IdleAction);
            EepromService::write<uint8_t>(_eepromAddrState, STATE_TARGETING_TEMPERATURE);
            DPRINTLN(F("[HeatingZone] HeatMode (IdleAction) - closing valves"));
        }
    }
}

void HeatingZone::_setup()
{
    _haHVAC->onModeCommand(onModeCommand);
    _haHVAC->onPowerCommand(onPowerCommand);
    _haHVAC->onTargetTemperatureCommand(onTargetTemperatureCommand);
    _haHVAC->setMinTemp(18.0f);
    _haHVAC->setMaxTemp(27.0f);
    _haHVAC->setTempStep(0.5f);
    _targetTemperature = EepromService::read<float>(_eepromAddrTargetTemperature);
    if (isnan(_targetTemperature) || _targetTemperature > 27 || _targetTemperature < 18)
    {
        _targetTemperature = 23;
        EepromService::write(_eepromAddrTargetTemperature, _targetTemperature);
    }
    _haHVAC->setTargetTemperature(_targetTemperature);

    auto state = static_cast<HeatingCircuitCurrentState>(EepromService::read<uint8_t>(
        _eepromAddrState));
    if (isnan(state) || state < 0 || state > 2)
    {
        state = STATE_POWERED_OFF;
        EepromService::write(_eepromAddrState, state);
    }

    switch (state)
    {
    case STATE_POWERED_OFF:
        _poweredOn = false;
        _mode = HAHVAC::OffMode;
        _haHVAC->setAction(HAHVAC::UnknownAction);
        _haHVAC->setMode(_mode);
        break;

    case STATE_MODE_OFF:
        _poweredOn = true;
        _mode = HAHVAC::OffMode;
        _haHVAC->setMode(_mode);
        break;

    case STATE_TARGETING_TEMPERATURE:
        _poweredOn = true;
        _mode = HAHVAC::HeatMode;
        _haHVAC->setMode(_mode);
        break;
    }

    DPRINT(F("[HeatingZone] (floor: "));
    DPRINT(_floor);
    DPRINT(F(" zone: "));
    DPRINT(_zoneNumber);
    DPRINT(F(") Initialized with mode: "));
    DPRINT(_mode);
    DPRINT(F(" and target temp: "));
    DPRINTLN(_targetTemperature);
}


void HeatingZone::_setAllValves(uint8_t state) const
{
    for (uint8_t i = 0; i < _valveCount; i++)
    {
        digitalWrite(_valvePins[i], state);
    }
}

HeatingZone* HeatingZone::findInstance(HAHVAC* haHVAC)
{
    for (HeatingZone* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->_haHVAC == haHVAC)
        {
            return current;
        }
    }
    return nullptr;
}

void HeatingZone::onTargetTemperatureCommand(HANumeric temperature, HAHVAC* sender)
{
    HeatingZone* heatingCircuit = HeatingZone::findInstance(sender);
    if (heatingCircuit == nullptr)
    {
        return;
    }
    float temperatureValue = temperature.toFloat();
    heatingCircuit->setTargetTemperature(temperatureValue);
}

void HeatingZone::onPowerCommand(bool state, HAHVAC* sender)
{
    HeatingZone* heatingCircuit = HeatingZone::findInstance(sender);
    if (heatingCircuit == nullptr)
    {
        return;
    }
    heatingCircuit->_poweredOn = state;
    DPRINT(F("[HeatingZone] Power state updated: "));
    DPRINTLN(state);
    heatingCircuit->_update();
}


void HeatingZone::onModeCommand(HAHVAC::Mode mode, HAHVAC* sender)
{
    HeatingZone* heatingCircuit = HeatingZone::findInstance(sender);
    if (heatingCircuit == nullptr)
    {
        return;
    }

    if (mode == HAHVAC::OffMode || mode == HAHVAC::HeatMode)
    {
        heatingCircuit->_mode = mode;
        heatingCircuit->_update();
        sender->setMode(mode);
        DPRINT(F("[HeatingZone] mode updated: "));
        DPRINTLN(mode);
    }
}
