//
// Created by zielq on 21.10.2025.
//

#include "FloorHeatingThermostat.h"

FloorHeatingThermostat* FloorHeatingThermostat::_head = nullptr;

void FloorHeatingThermostat::_setup()
{
    haHVAC->onModeCommand(onModeCommand);
    haHVAC->onTargetTemperatureCommand(onTargetTemperatureCommand);
    float targetTemperature = EepromService::read<uint8_t>(_eepromAddrTargetTemperature) / 2.0f;
    if (isnan(targetTemperature) || targetTemperature > MAX_TEMPERATURE || targetTemperature < MIN_TEMPERATURE)
    {
        targetTemperature = DEFAULT_TEMPERATURE;
        EepromService::write(_eepromAddrTargetTemperature, static_cast<uint8_t>(targetTemperature * 2));
    }
    haHVAC->setCurrentTargetTemperature(targetTemperature);

    auto mode = EepromService::read<uint8_t>(
        _eepromAddrState);
    if (mode != HAHVAC::AutoMode && mode != HAHVAC::OffMode)
    {
        mode = HAHVAC::OffMode;
        EepromService::write(_eepromAddrState, mode);
    }

    haHVAC->setCurrentMode(static_cast<HAHVAC::Mode>(mode));

    DPRINT(F("[FloorHeatingThermostat] ("));
    DPRINT(haHVAC->uniqueId());
    DPRINT(F(") initialized with targetTemperature: "));
    DPRINT(targetTemperature);
    DPRINT(F(" and mode: "));
    DPRINTLN(mode);
}

void FloorHeatingThermostat::setup()
{
    for (FloorHeatingThermostat* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_setup();
    }
    DPRINTLN(F("[FloorHeatingThermostat] All FloorHeatingThermostats initialized."));
}

bool FloorHeatingThermostat::isHeatNeeded()
{
    constexpr float HYSTERESIS = 0.5f;
    const float current = haHVAC->getCurrentTemperature().toFloat();
    const float target = haHVAC->getCurrentTargetTemperature().toFloat();
    return (current - target) < -HYSTERESIS;
}

FloorHeatingThermostat* FloorHeatingThermostat::findInstance(HAHVAC* haHVAC)
{
    for (FloorHeatingThermostat* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->haHVAC == haHVAC)
        {
            return current;
        }
    }
    return nullptr;
}

void FloorHeatingThermostat::onTargetTemperatureCommand(HANumeric temperature, HAHVAC* sender)
{
    FloorHeatingThermostat* thermostat = FloorHeatingThermostat::findInstance(sender);
    if (thermostat == nullptr)
    {
        return;
    }
    float temperatureValue = temperature.toFloat();
    thermostat->haHVAC->setTargetTemperature(temperatureValue);
    EepromService::write(thermostat->_eepromAddrTargetTemperature, static_cast<uint8_t>(temperatureValue) * 2);
    DPRINT(F("[FloorHeatingThermostat] ("));
    DPRINT(thermostat->haHVAC->uniqueId());
    DPRINT(F(") target temperature updated: "));
    DPRINTLN(temperatureValue);
}

void FloorHeatingThermostat::onModeCommand(HAHVAC::Mode mode, HAHVAC* sender)
{
    FloorHeatingThermostat* thermostat = FloorHeatingThermostat::findInstance(sender);
    if (thermostat == nullptr)
    {
        return;
    }

    if (mode == HAHVAC::OffMode || mode == HAHVAC::AutoMode)
    {
        thermostat->haHVAC->setMode(mode, true);
        EepromService::write(thermostat->_eepromAddrState, static_cast<uint8_t>(mode));
        DPRINT(F("[FloorHeatingThermostat] ("));
        DPRINT(thermostat->haHVAC->uniqueId());
        DPRINT(F(") mode updated: "));
        DPRINTLN(mode);
    }
}
