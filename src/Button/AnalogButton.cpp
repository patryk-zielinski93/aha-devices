#include "AnalogButton.h"

// The voltage threshold above which we consider a button to be pressed.
const uint8_t ANALOG_BUTTON_PRESS_THRESHOLD_V = 4;

uint8_t AnalogButton::getVoltage()
{
    // We use direct integer math for performance, based on Controllino's 24V spec (1 step = 30mV).
    // analogRead() is fast enough that we don't need complex averaging for this use case.
    int analogValue = analogRead(_pin);
    long millivolts = (long)analogValue * 30;
    return (uint8_t)((millivolts + 500) / 1000); // Convert to volts with rounding
}

void AnalogButton::loop()
{
    // Step 1: Read the current state
    uint8_t currentVoltage = getVoltage();
    bool isPressed = (currentVoltage >= ANALOG_BUTTON_PRESS_THRESHOLD_V);

    // Step 2: Generate event using the base class logic
    // The getButtonEvent() handles all complex timing for debounce, clicks, and long presses.
    ButtonEvent event = getButtonEvent(isPressed);

    // Step 3: CRITICAL LOGIC - Remember the voltage
    // If the button is physically pressed RIGHT NOW, we update our "memory" of the voltage.
    // If it's not pressed, we DON'T update it, so it holds the last valid press voltage.
    if (isPressed)
    {
        _samplesCount += 1;
        _samplesSum += currentVoltage;
    }

    // Step 4: Call the callback with the correct voltage
    if (event != BUTTON_EVENT_IDLE)
    {
        // For ANY event, we now report the voltage that we remembered from the press.
        // - For PRESSED/LONG_PRESSED, it was just updated with the current voltage.
        // - For RELEASED/CLICKED, it holds the value from just before the physical release.
        // This solves the problem entirely.
        _callback(event, _samplesSum / _samplesCount, this);
        _samplesCount = 0;
        _samplesSum = 0;
    }
}
