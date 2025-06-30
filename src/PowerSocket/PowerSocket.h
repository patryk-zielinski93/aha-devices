// Filename: PowerSocket.h
// Version: Refactored with Linked List, Two-Phase Init, and Memory Safety

#ifndef AHA_DEVICES_POWERSOCKET_H
#define AHA_DEVICES_POWERSOCKET_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"

/**
 * @class PowerSocket
 * @brief Represents a physical power socket controlled by a single digital pin.
 * Manages its state, handles auto-registration for HA callbacks, and ensures
 * memory-safe operation.
 */
class PowerSocket
{
public:
    // Constructor for PROGMEM (Flash) strings via F() macro
    PowerSocket(
        HASwitch* haSwitch,
        const __FlashStringHelper* name,
        uint8_t switchPin,
        uint8_t switchedOnPinState = HIGH, // Default to HIGH for standard relays
        const __FlashStringHelper* icon = nullptr
    );

    // Overloaded constructor for standard C-strings from SRAM
    PowerSocket(
        HASwitch* haSwitch,
        const char* name,
        uint8_t switchPin,
        uint8_t switchedOnPinState = HIGH,
        const char* icon = nullptr
    );

    // Destructor to free dynamically allocated memory for name and icon
    virtual ~PowerSocket();

    // Static setup method to configure all created instances
    static void setup();

    // Public API for controlling the socket state
    void setState(bool state);
    bool getState() const;
    void toggleState();

private:
    HASwitch* _haSwitch;
    uint8_t _switchPin;
    uint8_t _switchedOnPinState;
    bool _state;

    // Buffers for heap-allocated name/icon strings
    char* _haNameBuffer;
    char* _haIconBuffer;

    // Private initializer methods to avoid code duplication
    void _initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon);
    void _initialize(const char* name, const char* icon);

    // Private instance setup method
    void _setup();

    // --- Linked List for Instance Management ---
    PowerSocket* _nextInstance;
    static PowerSocket* _head;

    // --- Static callback handling ---
    static PowerSocket* _findInstance(HASwitch* haSwitch);
    static void _onSwitchCommand(bool state, HASwitch* sender);
};

#endif //AHA_DEVICES_POWERSOCKET_H
