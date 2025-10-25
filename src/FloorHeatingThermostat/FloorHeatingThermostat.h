//
// Created by zielq on 21.10.2025.
//

#ifndef AHA_DEVICES_FLOORHEATINGTHERMOSTAT_H
#define AHA_DEVICES_FLOORHEATINGTHERMOSTAT_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "EepromSerivce.h"
#include "FloorHeatingZone/HeatingZone.h"
#include "device-types/HAHVAC.h"

class FloorHeatingThermostat

{
public:
    static constexpr uint16_t features = HAHVAC::TargetTemperatureFeature | HAHVAC::ModesFeature |
        HAHVAC::ActionFeature;
    static constexpr float MAX_TEMPERATURE = 25.0f;
    static constexpr float MIN_TEMPERATURE = 18.0f;
    static constexpr float TEMPERATURE_STEP = 0.5f;
    static constexpr float DEFAULT_TEMPERATURE = 21.5f;

    /**
     * @brief Inicjalizuje wszystkie termostaty. Wywołać w setup().
     * @param zones Wskaźnik do globalnej tablicy stref.
     * @param zoneCount Liczba stref.
     */
    static void setup(HeatingZone* zones, uint8_t zoneCount);

    /**
     * @brief Sprawdza, czy strefa potrzebuje grzania (z uwzględnieniem histerezy).
     */
    static bool isHeatNeeded(const HeatingZone& zone);

private:
    /**
     * @brief Wewnętrzna metoda setup dla pojedynczej strefy.
     */
    static void _setup(HeatingZone& zone);

    // --- Wskaźniki do globalnej tablicy stref (dla callbacków) ---
    static HeatingZone* _zones;
    static uint8_t _zoneCount;

    // --- Statyczne callbacki ---
    static HeatingZone* findInstance(HAHVAC* haHVAC);
    static void onTargetTemperatureCommand(HANumeric temperature, HAHVAC* sender);
    static void onModeCommand(HAHVAC::Mode mode, HAHVAC* sender);
};


#endif //AHA_DEVICES_FLOORHEATINGTHERMOSTAT_H
