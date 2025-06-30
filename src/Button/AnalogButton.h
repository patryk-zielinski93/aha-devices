#ifndef AHA_DEVICES_ANALOGBUTTON_H
#define AHA_DEVICES_ANALOGBUTTON_H

#include "Button.h"

class AnalogButton;
typedef void (*AnalogButtonCallback)(ButtonEvent event, uint8_t voltage, AnalogButton* caller);

class AnalogButton : public Button
{
private:
    AnalogButtonCallback _callback;
    ButtonEvent _lastEvent = BUTTON_EVENT_IDLE;
    uint8_t _voltageWhenPressed;
    uint8_t getVoltage();

public:
    AnalogButton(uint16_t id, uint8_t pin, AnalogButtonCallback callback)
        : Button(id, pin),
          _callback(callback),
          _voltageWhenPressed(0)
    {
    }

    void loop() override;
};
#endif
