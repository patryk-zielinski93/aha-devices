//
// Created by zielq on 21.10.2025.
//

#include "Valve.h"

void Valve::setup(HeatingZone* zones, uint8_t zoneCount)
{
    for (uint8_t i = 0; i < zoneCount; i++)
    {
        HeatingZone& zone = zones[i];
        pinMode(zone.valvePin, OUTPUT);

        // Ustaw stan początkowy (NO - Normalnie Otwarty)
        // Otwarty = LOW (brak zasilania), Zamknięty = HIGH (zasilanie)
        if (zone.valveCurrentState == VALVE_STATE_CLOSED || zone.valveCurrentState == VALVE_STATE_CLOSING)
        {
            digitalWrite(zone.valvePin, HIGH);
        }
        else
        {
            digitalWrite(zone.valvePin, LOW);
        }
    }
}

void Valve::loop(HeatingZone* zones, uint8_t zoneCount)
{
    for (uint8_t i = 0; i < zoneCount; i++)
    {
        _loop(zones[i]);
    }
}

void Valve::open(HeatingZone& zone)
{
    // Jeśli już otwarty, nic nie rób
    if (zone.valveCurrentState == ValveState::VALVE_STATE_OPEN || zone.valveCurrentState ==
        ValveState::VALVE_STATE_OPENING)
    {
        return;
    }

    unsigned long now = millis();

    if (zone.valveCurrentState == ValveState::VALVE_STATE_CLOSED)
    {
        // Start otwierania od stanu zamkniętego: pełny czas
        zone.valveCurrentState = ValveState::VALVE_STATE_OPENING;
        zone.valveStateChangeFinishedAt = now + STATE_CHANGE_DURATION_MS;

        // Odłącz zasilanie (NO -> brak zasilania to otwieranie/otwarty)
        digitalWrite(zone.valvePin, LOW);

        DPRINT(F("[Valve ("));
        DPRINT(zone.valvePin);
        DPRINT(F(")] OPEN start: full duration ms="));
        DPRINTLN(STATE_CHANGE_DURATION_MS);

        return;
    }

    if (zone.valveCurrentState == ValveState::VALVE_STATE_CLOSING)
    {
        // Odwrócenie: nowy czas do otwarcia = czas, który upłynął w zamykaniu
        unsigned long remaining = (zone.valveStateChangeFinishedAt > now) ? (zone.valveStateChangeFinishedAt - now) : 0;
        unsigned long elapsed = STATE_CHANGE_DURATION_MS > remaining ? (STATE_CHANGE_DURATION_MS - remaining) : 0;

        zone.valveCurrentState = ValveState::VALVE_STATE_OPENING;
        zone.valveStateChangeFinishedAt = now + elapsed;

        // Odwracamy kierunek -> brak zasilania
        digitalWrite(zone.valvePin, LOW);

        DPRINT(F("[Valve ("));
        DPRINT(zone.valvePin);
        DPRINT(F(")] OPEN reversed: elapsed ms="));
        DPRINT(elapsed);
        DPRINT(F(" remaining was="));
        DPRINTLN(remaining);

        return;
    }
}

void Valve::close(HeatingZone& zone)
{
    // Jeśli już zamknięty, nic nie rób
    if (zone.valveCurrentState == ValveState::VALVE_STATE_CLOSED || zone.valveCurrentState ==
        ValveState::VALVE_STATE_CLOSING)
    {
        return;
    }

    unsigned long now = millis();

    if (zone.valveCurrentState == ValveState::VALVE_STATE_OPEN)
    {
        // Start zamykania od stanu otwartego: pełny czas
        zone.valveCurrentState = ValveState::VALVE_STATE_CLOSING;
        zone.valveStateChangeFinishedAt = now + STATE_CHANGE_DURATION_MS;

        // Podaj zasilanie (NO -> zasilanie zamyka)
        digitalWrite(zone.valvePin, HIGH);

        DPRINT(F("[Valve ("));
        DPRINT(zone.valvePin);
        DPRINTLN(F(")] CLOSE start"));

        return;
    }

    if (zone.valveCurrentState == ValveState::VALVE_STATE_OPENING)
    {
        // Odwrócenie: nowy czas do zamknięcia = czas, który upłynął w otwieraniu
        unsigned long remaining = (zone.valveStateChangeFinishedAt > now) ? (zone.valveStateChangeFinishedAt - now) : 0;
        unsigned long elapsed = STATE_CHANGE_DURATION_MS > remaining ? (STATE_CHANGE_DURATION_MS - remaining) : 0;

        zone.valveCurrentState = ValveState::VALVE_STATE_CLOSING;
        zone.valveStateChangeFinishedAt = now + elapsed;

        // Podaj zasilanie
        digitalWrite(zone.valvePin, HIGH);

        DPRINT(F("[Valve ("));
        DPRINT(zone.valvePin);
        DPRINT(F(")] CLOSE reversed: elapsed ms="));
        DPRINT(elapsed);
        DPRINT(F(" remaining was="));
        DPRINTLN(remaining);

        return;
    }
}

ValveState Valve::getState(const HeatingZone& zone)
{
    return zone.valveCurrentState;
}

void Valve::_loop(HeatingZone& zone)
{
    unsigned long now = millis();

    if (zone.valveCurrentState == ValveState::VALVE_STATE_OPENING || zone.valveCurrentState ==
        ValveState::VALVE_STATE_CLOSING)
    {
        // Sprawdź, czy zakończył się ruch
        if (now >= zone.valveStateChangeFinishedAt)
        {
            if (zone.valveCurrentState == ValveState::VALVE_STATE_OPENING)
            {
                zone.valveCurrentState = ValveState::VALVE_STATE_OPEN;
                digitalWrite(zone.valvePin, LOW); // Upewnij się, że zasilanie jest wyłączone

                DPRINT(F("[Valve ("));
                DPRINT(zone.valvePin);
                DPRINTLN(F(")] OPEN completed"));
            }
            else
            {
                zone.valveCurrentState = ValveState::VALVE_STATE_CLOSED;
                digitalWrite(zone.valvePin, HIGH); // Upewnij się, że zasilanie jest włączone

                DPRINT(F("[Valve ("));
                DPRINT(zone.valvePin);
                DPRINTLN(F(")] CLOSE completed"));
            }

            // Ustabilizowane: koniec ruchu
            zone.valveStateChangeFinishedAt = 0;
        }
    }
}
