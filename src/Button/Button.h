//
// Created by zielq on 21.11.2024.
//

#ifndef AHA_DEVICES_BUTTON_H
#define AHA_DEVICES_BUTTON_H

#include <Arduino.h>

enum ButtonEvent : uint8_t
{
    BUTTON_EVENT_IDLE,
    BUTTON_EVENT_RELEASED,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_DOUBLE_PRESSED,
    BUTTON_EVENT_LONG_PRESSED,
    BUTTON_EVENT_CLICKED,
    BUTTON_EVENT_DOUBLE_CLICKED,
    BUTTON_EVENT_TRIPLE_CLICKED
};

class Button
{
protected:
    static const uint8_t _buttonDebounceDelayMs = 5;
    static const uint16_t _buttonPressDelayMs = 350;
    static const uint16_t _buttonLongPressDelayMs = 1200;
    static const uint16_t _buttonClickDelayMs = 375;

    bool _lastButtonState = false;
    uint8_t _clickCounter = 0;
    unsigned long _debounceTime = 0;
    bool _pressing = false;
    bool _pressEventSent = false;
    bool _longPressEventSent = false;
    bool _doublePressEventSent = false;

    uint16_t _id;
    uint8_t _pin;

public:
    explicit Button(uint16_t id, uint8_t _pin) : _id(id), _pin(_pin)
    {
    }

    virtual void loop() = 0;

    ButtonEvent getButtonEvent(bool isPressed);

    void reset()
    {
        _clickCounter = 0;
        _debounceTime = 0;
        _lastButtonState = false;
        _pressEventSent = false;
        _longPressEventSent = false;
        _doublePressEventSent = false;
        _pressing = false;
    }

    void setup() const
    {
        pinMode(_pin, INPUT);
    }

    uint8_t getPin() const
    {
        return _pin;
    }

    uint16_t getId() const
    {
        return _id;
    }
};

#endif //AHA_DEVICES_BUTTON_H
