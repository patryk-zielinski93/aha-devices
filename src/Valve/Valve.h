//
// Created by zielq on 21.10.2025.
//

#ifndef AHA_DEVICES_VALVE_H
#define AHA_DEVICES_VALVE_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "EepromSerivce.h"
#include "FloorHeatingZone/HeatingZone.h"

class Valve
{
public:
    // Czas zmiany stanu jest teraz stałą, nie zajmuje RAM w obiekcie
    static const unsigned long STATE_CHANGE_DURATION_MS = 120000;

    /**
     * @brief Inicjalizuje piny zaworów (pinMode, initial state). Wywołać w setup().
     */
    static void setup(HeatingZone* zones, uint8_t zoneCount);

    /**
     * @brief Pętla do obsługi stanów przejściowych zaworów (otwieranie/zamykanie).
     */
    static void loop(HeatingZone* zones, uint8_t zoneCount);

    /**
     * @brief Rozpoczyna proces otwierania zaworu dla danej strefy.
     */
    static void open(HeatingZone& zone);

    /**
     * @brief Rozpoczyna proces zamykania zaworu dla danej strefy.
     */
    static void close(HeatingZone& zone);

    /**
     * @brief Zwraca aktualny stan zaworu dla danej strefy.
     */
    static ValveState getState(const HeatingZone& zone);

private:
    /**
     * @brief Wewnętrzna pętla logiki dla pojedynczego zaworu.
     */
    static void _loop(HeatingZone& zone);
};


#endif //AHA_DEVICES_VALVE_H
