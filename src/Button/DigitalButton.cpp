//
// Created by zielq on 21.11.2024.
//

#include "DigitalButton.h"

void DigitalButton::loop()
{
    bool isPressed = digitalRead(_pin) == _pressedState;
    ButtonEvent event = getButtonEvent(isPressed);

    if (event != BUTTON_EVENT_IDLE && event != _lastEvent)
    {
        _callback(event, this);
        _lastEvent = event;
    }
}
