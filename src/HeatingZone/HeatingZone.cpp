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

void HeatingZone::updateHAStates()
{
    for (HeatingZone* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_haHVAC->setTargetTemperature(current->getCurrentTemperature(), true);
        current->_haHVAC->setAction(current->_currentAction, true);
        current->_haHVAC->setMode(current->_mode, true);
    }
}


void HeatingZone::setCurrentTemperature(float temperature)
{
    if (_currentTemperature == temperature || temperature == -127.0f)
    {
        _update();
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

void HeatingZone::setTargetTemperature(float temperature)
{
    if (_targetTemperature == temperature)
    {
        _update();
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
    return _currentAction == HAHVAC::HeatingAction &&
        (_valveState == LOW && millis() - _valveStateChange > valveStateChangeDuration);
}

void HeatingZone::_update()
{
    if (_valvePins == nullptr || _valveCount == 0)
    {
        DPRINTLN(F("[HeatingController#update] No valves configured"));
        return;
    }

    if (_mode == HAHVAC::OffMode)
    {
        if (_currentAction == HAHVAC::OffAction)
        {
            DPRINTLN(F("[HeatingZone#update] OffMode and OffAction active. Skipping"));
            return;
        }
        _setAllValves(HIGH);
        _setAction(HAHVAC::OffAction);
        EepromService::write<uint8_t>(_eepromAddrState, STATE_MODE_OFF);
        DPRINTLN(F("[HeatingZone#update] OffMode activated - closing valves"));
        return;
    }

    if (_mode == HAHVAC::HeatMode)
    {
        if (_currentTemperature == 0)
        {
            DPRINTLN(F("[HeatingController#update] Current temperature is not initialized"));
            return;
        }
        // Add hysteresis to prevent frequent switching
        const float hysteresis = 0.25f; // 0.25°C hysteresis

        // Determine if heating is needed based on hysteresis
        bool shouldHeat = false;
        bool shouldStop = false;

        if (_currentAction == HAHVAC::HeatingAction)
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

        DPRINT(F("[HeatingZone#update] HeatMode evaluation:"));
        DPRINT(F(" currentTemp: "));
        DPRINT(_currentTemperature);
        DPRINT(F(" targetTemp: "));
        DPRINT(_targetTemperature);
        DPRINT(F(" shouldHeat: "));
        DPRINT(shouldHeat);
        DPRINT(F(" shouldStop: "));
        DPRINT(shouldStop);
        DPRINT(F(" isHeatNeeded: "));
        DPRINT(isHeatNeeded());
        DPRINT(F(" _valveStateChange: "));
        DPRINTLN(_valveStateChange);

        if (shouldHeat && _currentAction != HAHVAC::HeatingAction)
        {
            _setAllValves(LOW);
            _setAction(HAHVAC::HeatingAction);
            EepromService::write<uint8_t>(_eepromAddrState, STATE_MODE_HEAT);
            DPRINTLN(F("[HeatingZone#update] HeatMode (HeatingAction) - opening valves"));
        }
        else if (shouldStop && _currentAction != HAHVAC::IdleAction)
        {
            _setAllValves(HIGH);
            _setAction(HAHVAC::IdleAction);
            EepromService::write<uint8_t>(_eepromAddrState, STATE_MODE_HEAT);
            DPRINTLN(F("[HeatingZone#update] HeatMode (IdleAction) - closing valves"));
        }
    }
    DPRINTLN(F("[HeatingZone#update] executed"));
}

void HeatingZone::_setup()
{
    _haHVAC->onModeCommand(onModeCommand);
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
    _haHVAC->setTargetTemperature(_targetTemperature, true);

    auto state = static_cast<HeatingCircuitCurrentState>(EepromService::read<uint8_t>(
        _eepromAddrState));
    if (isnan(state) || state < 0 || state > 1)
    {
        state = STATE_MODE_OFF;
        EepromService::write(_eepromAddrState, state);
    }

    switch (state)
    {
    case STATE_MODE_OFF:
        _setMode(HAHVAC::OffMode);
        break;

    case STATE_MODE_HEAT:
        _setMode(HAHVAC::HeatMode);
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


void HeatingZone::_setAllValves(uint8_t state)
{
    bool updated = false;
    for (uint8_t i = 0; i < _valveCount; i++)
    {
        if (digitalRead(_valvePins[i]) != state)
        {
            digitalWrite(_valvePins[i], state);
            updated = true;
        }
    }
    if (updated)
    {
        _valveStateChange = millis();
        _valveState = state;
    }
}

void HeatingZone::_setAction(HAHVAC::Action action)
{
    _haHVAC->setAction(action);
    _currentAction = action;
}

void HeatingZone::_setMode(HAHVAC::Mode mode)
{
    _haHVAC->setMode(mode);
    _mode = mode;
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

void HeatingZone::onModeCommand(HAHVAC::Mode mode, HAHVAC* sender)
{
    HeatingZone* heatingCircuit = HeatingZone::findInstance(sender);
    if (heatingCircuit == nullptr)
    {
        return;
    }

    if (mode == HAHVAC::OffMode || mode == HAHVAC::HeatMode)
    {
        heatingCircuit->_setMode(mode);
        heatingCircuit->_update();
        DPRINT(F("[HeatingZone] mode updated: "));
        DPRINTLN(mode);
    }
}
