//
// Created by zielq on 21.11.2024.
//

#include "Button.h"


ButtonEvent Button::getButtonEvent(bool buttonState)
{
    ButtonEvent event = BUTTON_EVENT_IDLE;

    // If the switch changed, due to noise or pressing:
    if (buttonState != _lastButtonState)
    {
        // reset the debouncing timer
        _debounceTime = millis();
    }

    uint16_t timeDiff = millis() - _debounceTime;
    if (timeDiff > _buttonDebounceDelayMs)
    {
        // whatever the reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state

        if (!buttonState)
        {
            // button is NOT pressed
            if (_pressing)
            {
                // Only generate a RELEASED event if a PRESSED or LONG_PRESSED or DOUBLE_LONG_PRESSED event has been previously sent.
                // This ensures that releasing the button after a short click will not trigger this event.
                if (_pressEventSent || _longPressEventSent || _doublePressEventSent)
                {
                    event = BUTTON_EVENT_RELEASED;
                }

                if (timeDiff < _buttonClickDelayMs && !_pressEventSent)
                {
                    _clickCounter++;
                }

                _pressEventSent = false;
                _longPressEventSent = false;
                _doublePressEventSent = false;
                _pressing = false;
            }

            if ((_clickCounter > 0 && timeDiff > _buttonClickDelayMs) || _clickCounter == 3)
            {
                if (_clickCounter == 1)
                {
                    event = BUTTON_EVENT_CLICKED;
                }
                else if (_clickCounter == 2)
                {
                    event = BUTTON_EVENT_DOUBLE_CLICKED;
                }
                else if (_clickCounter == 3)
                {
                    event = BUTTON_EVENT_TRIPLE_CLICKED;
                }

                _clickCounter = 0;
            }
        }
        else
        {
            // button is pressed
            if (!_pressEventSent && timeDiff > _buttonPressDelayMs)
            {
                if (_clickCounter == 1)
                {
                    event = BUTTON_EVENT_DOUBLE_PRESSED;
                    _doublePressEventSent = true;
                }
                else
                {
                    event = BUTTON_EVENT_PRESSED;
                }
                _clickCounter = 0;
                _pressEventSent = true;
            }

            if (!_doublePressEventSent && !_longPressEventSent && timeDiff > _buttonLongPressDelayMs)
            {
                event = BUTTON_EVENT_LONG_PRESSED;
                _longPressEventSent = true;
            }

            if (!_pressing)
            {
                _pressing = true;
            }
        }
    }

    _lastButtonState = buttonState;

    return event;
}
