//
// Created by Patryk Zieliński on 22/07/2024.
//

#ifndef AHA_DEVICES_LIGHT_H
#define AHA_DEVICES_LIGHT_H

#include <Arduino.h>
#include <ArduinoHA.h>

#include "Debug.h"

/**
 * @class Light
 * @brief Represents a physical light, manages its state, and handles auto-registration.
 * This implementation uses a linked list for instance management and ensures safe,
 * two-phase initialization for compatibility with the ArduinoHA library.
 */
class Light {
public:
    /**
     * @brief Constructor for using PROGMEM (Flash) strings via F() macro.
     * @param pin The GPIO pin number controlling the light.
     * @param haLight The HALight entity from the ArduinoHA library.
     * @param name The name of the light for Home Assistant (use F() macro).
     * @param icon Optional: The icon for Home Assistant (use F() macro).
     */
    Light(
        uint8_t pin,
        HALight* haLight,
        const __FlashStringHelper* name = nullptr,
        const __FlashStringHelper* icon = nullptr
    );

    /**
     * @brief Overloaded constructor for using standard C-strings from SRAM.
     * @param pin The GPIO pin number controlling the light.
     * @param haLight The HALight entity from the ArduinoHA library.
     * @param name The name of the light for Home Assistant.
     * @param icon Optional: The icon for Home Assistant.
     */
    Light(
        uint8_t pin,
        HALight* haLight,
        const char* name,
        const char* icon = nullptr
    );

    /**
     * @brief Destructor to free dynamically allocated memory for name and icon.
     */
    virtual ~Light();

    /**
     * @brief A static method that iterates through all created Light instances
     * and calls their individual _setup() method.
     */
    static void setup();

    // Public API for controlling the light
    void setState(bool state);
    void toggleState();
    void turnOn();
    void turnOff();
    bool getState() const;

private:
    uint8_t _pin;
    bool _currentState;
    HALight* _haLight;

    // Buffers to hold the persistent heap-allocated copies of name and icon
    char* _haNameBuffer;
    char* _haIconBuffer;

    /**
     * @brief Private initializer for PROGMEM strings.
     */
    void _initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon);

    /**
     * @brief Private initializer for SRAM strings.
     */
    void _initialize(const char* name, const char* icon);

    /**
     * @brief Performs the actual setup for an instance (pin mode, callback registration).
     */
    void _setup();

    // --- Linked List for Instance Management ---
    Light* _nextInstance; // Pointer to the next Light object in the list
    static Light* _head; // Static pointer to the start of the list

    // --- Static callback handling ---
    static Light* findInstance(HALight* haLight);
    static void onStateCommand(bool state, HALight* sender);
};


#endif //AHA_DEVICES_LIGHT_H
