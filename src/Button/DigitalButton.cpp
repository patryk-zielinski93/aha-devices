//
// Created by zielq on 21.11.2024.
//

#include "DigitalButton.h"

void DigitalButton::loop()
{
    bool isPressed = digitalRead(_pin) == _pressedState;
    ButtonEvent event = getButtonEvent(isPressed);

    // The check for _lastEvent is removed. We trust getButtonEvent.
    if (event != BUTTON_EVENT_IDLE)
    {
        _callback(event, this);
    }
}
