//
// Created by zielq on 21.11.2024.
//

#ifndef AHA_DEVICES_DIGITALBUTTON_H
#define AHA_DEVICES_DIGITALBUTTON_H

#include "Button.h"

class DigitalButton;

typedef void (*DigitalButtonCallback)(ButtonEvent event, DigitalButton* caller);

class DigitalButton : public Button {
private:
    DigitalButtonCallback _callback;
    bool _pressedState; // LOW / HIGH

public:
    DigitalButton(uint16_t id, uint8_t pin, DigitalButtonCallback callback, bool pressedState = HIGH)
        : Button(id, pin), _callback(callback), _pressedState(pressedState) {}

    void loop() override;
};


#endif //AHA_DEVICES_DIGITALBUTTON_H
