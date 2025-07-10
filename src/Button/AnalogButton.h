#ifndef AHA_DEVICES_ANALOGBUTTON_H
#define AHA_DEVICES_ANALOGBUTTON_H

#include "Button.h"

class AnalogButton;
typedef void (*AnalogButtonCallback)(ButtonEvent event, uint8_t voltage, AnalogButton* caller);

class AnalogButton : public Button
{
private:
    AnalogButtonCallback _callback;
    uint32_t _samplesCount;
    uint32_t _samplesSum;
    uint8_t getVoltage();

public:
    AnalogButton(uint16_t id, uint8_t pin, AnalogButtonCallback callback)
        : Button(id, pin),
          _callback(callback),
          _samplesCount(0),
          _samplesSum(0)
    {
    }

    void loop() override;
};
#endif
