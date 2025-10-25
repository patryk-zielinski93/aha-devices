//
// Created by zielq on 21.10.2025.
//

#include "FloorHeatingThermostat.h"

// Inicjalizacja statycznych wskaźników
HeatingZone* FloorHeatingThermostat::_zones = nullptr;
uint8_t FloorHeatingThermostat::_zoneCount = 0;


void FloorHeatingThermostat::_setup(HeatingZone& zone)
{
    // --- Logika przeniesiona z konstruktora ---
    zone.haHVAC->setName(zone.haHVAC->uniqueId()); // Domyślna nazwa
    // haHVAC->setIcon(icon); // Pomijamy opcjonalne
    zone.haHVAC->setModes(HAHVAC::AutoMode | HAHVAC::OffMode);
    zone.haHVAC->setMinTemp(MIN_TEMPERATURE);
    zone.haHVAC->setMaxTemp(MAX_TEMPERATURE);
    zone.haHVAC->setTempStep(TEMPERATURE_STEP);
    zone.haHVAC->setAction(HAHVAC::Action::IdleAction);
    zone.haHVAC->setCurrentCurrentTemperature(0);
    // --- Koniec logiki z konstruktora ---

    zone.haHVAC->onModeCommand(onModeCommand);
    zone.haHVAC->onTargetTemperatureCommand(onTargetTemperatureCommand);

    float targetTemperature = EepromService::read<uint8_t>(zone.eepromAddrTargetTemperature) / 2.0f;
    if (isnan(targetTemperature) || targetTemperature > MAX_TEMPERATURE || targetTemperature < MIN_TEMPERATURE)
    {
        targetTemperature = DEFAULT_TEMPERATURE;
        EepromService::write(zone.eepromAddrTargetTemperature, static_cast<uint8_t>(targetTemperature * 2));
    }
    zone.haHVAC->setCurrentTargetTemperature(targetTemperature);

    auto mode = EepromService::read<uint8_t>(zone.eepromAddrState);
    if (mode != HAHVAC::AutoMode && mode != HAHVAC::OffMode)
    {
        mode = HAHVAC::OffMode;
        EepromService::write(zone.eepromAddrState, mode);
    }

    zone.haHVAC->setCurrentMode(static_cast<HAHVAC::Mode>(mode));

    DPRINT(F("[FloorHeatingThermostat] ("));
    DPRINT(zone.haHVAC->uniqueId());
    DPRINT(F(") initialized with targetTemperature: "));
    DPRINT(targetTemperature);
    DPRINT(F(" and mode: "));
    DPRINTLN(mode);
}

void FloorHeatingThermostat::setup(HeatingZone* zones, uint8_t zoneCount)
{
    // Zapisz globalny dostęp do tablicy stref dla callbacków
    _zones = zones;
    _zoneCount = zoneCount;

    for (uint8_t i = 0; i < _zoneCount; i++)
    {
        _setup(zones[i]);
    }
    DPRINTLN(F("[FloorHeatingThermostat] All FloorHeatingThermostats initialized."));
}

bool FloorHeatingThermostat::isHeatNeeded(const HeatingZone& zone)
{
    constexpr float HYSTERESIS = 0.5f;
    const float current = zone.haHVAC->getCurrentTemperature().toFloat();
    const float target = zone.haHVAC->getCurrentTargetTemperature().toFloat();
    return (current - target) < -HYSTERESIS;
}

HeatingZone* FloorHeatingThermostat::findInstance(HAHVAC* haHVAC)
{
    if (_zones == nullptr) return nullptr;

    for (uint8_t i = 0; i < _zoneCount; i++)
    {
        if (_zones[i].haHVAC == haHVAC)
        {
            return &_zones[i];
        }
    }
    return nullptr;
}

void FloorHeatingThermostat::onTargetTemperatureCommand(HANumeric temperature, HAHVAC* sender)
{
    HeatingZone* zone = FloorHeatingThermostat::findInstance(sender);
    if (zone == nullptr)
    {
        return;
    }
    float temperatureValue = temperature.toFloat();
    zone->haHVAC->setTargetTemperature(temperatureValue);
    EepromService::write(zone->eepromAddrTargetTemperature, static_cast<uint8_t>(temperatureValue) * 2);
    DPRINT(F("[FloorHeatingThermostat] ("));
    DPRINT(zone->haHVAC->uniqueId());
    DPRINT(F(") target temperature updated: "));
    DPRINTLN(temperatureValue);
}

void FloorHeatingThermostat::onModeCommand(HAHVAC::Mode mode, HAHVAC* sender)
{
    HeatingZone* zone = FloorHeatingThermostat::findInstance(sender);
    if (zone == nullptr)
    {
        return;
    }

    if (mode == HAHVAC::OffMode || mode == HAHVAC::AutoMode)
    {
        zone->haHVAC->setMode(mode, true);
        EepromService::write(zone->eepromAddrState, static_cast<uint8_t>(mode));
        DPRINT(F("[FloorHeatingThermostat] ("));
        DPRINT(zone->haHVAC->uniqueId());
        DPRINT(F(") mode updated: "));
        DPRINTLN(mode);
    }
}
