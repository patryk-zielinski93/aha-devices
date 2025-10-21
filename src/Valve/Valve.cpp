//
// Created by zielq on 21.10.2025.
//

#include "Valve.h"

Valve* Valve::_head = nullptr;

void Valve::loop()
{
    for (Valve* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_loop();
    }
}

void Valve::open()
{
    // Jeśli już otwarty, nic nie rób
    if (_currentState == ValveState::VALVE_STATE_OPEN || _currentState == ValveState::VALVE_STATE_OPENING)
    {
        return;
    }

    unsigned long now = millis();

    if (_currentState == ValveState::VALVE_STATE_CLOSED)
    {
        // Start otwierania od stanu zamkniętego: pełny czas
        _currentState = ValveState::VALVE_STATE_OPENING;
        _stateChangeFinishedAt = now + _stateChangeDurationMs;

        // Odłącz zasilanie (NO -> brak zasilania to otwieranie/otwarty)
        digitalWrite(_valvePin, LOW);

        DPRINT(F("[Valve] OPEN start: full duration ms="));
        DPRINTLN(_stateChangeDurationMs);

        return;
    }

    if (_currentState == ValveState::VALVE_STATE_CLOSING)
    {
        // Odwrócenie: nowy czas do otwarcia = czas, który upłynął w zamykaniu
        unsigned long remaining = (_stateChangeFinishedAt > now) ? (_stateChangeFinishedAt - now) : 0;
        unsigned long elapsed = _stateChangeDurationMs > remaining ? (_stateChangeDurationMs - remaining) : 0;

        _currentState = ValveState::VALVE_STATE_OPENING;
        _stateChangeFinishedAt = now + elapsed;

        // Odwracamy kierunek -> brak zasilania
        digitalWrite(_valvePin, LOW);

        DPRINT(F("[Valve] OPEN reversed: elapsed ms="));
        DPRINT(elapsed);
        DPRINT(F(" remaining was="));
        DPRINTLN(remaining);

        return;
    }
}

void Valve::close()
{
    // Jeśli już zamknięty, nic nie rób
    if (_currentState == ValveState::VALVE_STATE_CLOSED || _currentState == ValveState::VALVE_STATE_CLOSING)
    {
        return;
    }

    unsigned long now = millis();

    if (_currentState == ValveState::VALVE_STATE_OPEN)
    {
        // Start zamykania od stanu otwartego: pełny czas
        _currentState = ValveState::VALVE_STATE_CLOSING;
        _stateChangeFinishedAt = now + _stateChangeDurationMs;

        // Podaj zasilanie (NO -> zasilanie zamyka)
        digitalWrite(_valvePin, HIGH);

        DPRINT(F("[Valve ("));
        DPRINT(_valvePin);
        DPRINTLN(F(")] CLOSE start"));

        return;
    }

    if (_currentState == ValveState::VALVE_STATE_OPENING)
    {
        // Odwrócenie: nowy czas do zamknięcia = czas, który upłynął w otwieraniu
        unsigned long remaining = (_stateChangeFinishedAt > now) ? (_stateChangeFinishedAt - now) : 0;
        unsigned long elapsed = _stateChangeDurationMs > remaining ? (_stateChangeDurationMs - remaining) : 0;

        _currentState = ValveState::VALVE_STATE_CLOSING;
        _stateChangeFinishedAt = now + elapsed;

        // Podaj zasilanie
        digitalWrite(_valvePin, HIGH);

        DPRINT(F("[Valve] CLOSE reversed: elapsed ms="));
        DPRINT(elapsed);
        DPRINT(F(" remaining was="));
        DPRINTLN(remaining);

        return;
    }
}

ValveState Valve::getState()
{
    return _currentState;
}

void Valve::_loop()
{
    unsigned long now = millis();

    if (_currentState == ValveState::VALVE_STATE_OPENING || _currentState == ValveState::VALVE_STATE_CLOSING)
    {
        // Sprawdź, czy zakończył się ruch
        if (now >= _stateChangeFinishedAt)
        {
            if (_currentState == ValveState::VALVE_STATE_OPENING)
            {
                _currentState = ValveState::VALVE_STATE_OPEN;

                // Dla OPEN pin ma być LOW (brak zasilania)
                digitalWrite(_valvePin, LOW);

                DPRINTLN(F("[Valve] OPEN completed"));
            }
            else
            {
                _currentState = ValveState::VALVE_STATE_CLOSED;

                // Dla CLOSED pin ma być HIGH (ciągłe zasilanie)
                digitalWrite(_valvePin, HIGH);

                DPRINTLN(F("[Valve] CLOSE completed"));
            }

            // Ustabilizowane: koniec ruchu – nie używamy timera
            _stateChangeFinishedAt = 0;
        }
    }
}
