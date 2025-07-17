//
// Created by zielq on 25.03.2025.
//

#ifndef LEDSTRIP_H
#define LEDSTRIP_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include <ModbusRTUMaster.h>

#include "Debug.h"

constexpr uint8_t REGS_PER_GROUP = 12;
/**
 * @struct LedStripRegisterSet
 * @brief Opakowuje tablicę rejestrów w jeden obiekt.
 * Dzięki temu EepromService może traktować cały zestaw jako pojedynczą wartość
 * i stosować do niego mechanizm wear-leveling.
 */
struct LedStripRegisterSet
{
    uint16_t values[REGS_PER_GROUP] = {0}; // Inicjalizacja domyślna
};

enum LedStripState
{
    STATE_IDLE_OFF = 0,
    STATE_READY_TO_TURN_ON = 1,
    STATE_ON = 2,
    STATE_READY_TO_TURN_OFF = 3
};

enum LedStripCommand
{
    CMD_TURN_OFF_IDLE = 0, // Komenda: Wyłączone / bezczynne
    CMD_PREPARE_TURN_ON = 1, // Komenda: Przygotuj do włączenia
    CMD_TURN_ON = 2, // Komenda: Przekaźnik został włączony, włącz taśmę
    CMD_PREPARE_TURN_OFF = 3 // Komenda: Przygotuj do wyłączenia
};

enum LedStripRegister
{
    // Rejestry zapisywane przez Mastera
    REG_STATE_COMMAND = 0, // Master zapisuje tu komendy
    REG_STATE_STATUS = 1, // Slave raportuje tutaj swój obecny stan
    REG_MODE = 2, // 0: Kolor statyczny, 1: Animacja
    REG_BRIGHTNESS = 3, // 0-255
    REG_R = 4, // 0-255
    REG_G = 5, // 0-255
    REG_B = 6, // 0-255
    REG_CW = 7, // 0-255 Zimny biały
    REG_WW = 8, // 0-255 Ciepły biały
    REG_ANIMATION_ID = 9, // ID animacji do uruchomienia
    REG_ANIMATION_SPEED = 10, // Prędkość animacji (1-255)
    REG_COLOR_CYCLE = 11 // 0: Wył, 1: Wł (dla animacji)
};

class LedStrip
{
public:
    static void setup();
    static void loop();

    LedStrip(
        ModbusRTUMaster* modbusMaster,
        HALight* haLight,
        const __FlashStringHelper* name,
        uint8_t pin,
        int ledStripIndexInController,
        uint16_t eepromAddr,
        uint8_t eepromSlots,
        const __FlashStringHelper* icon = nullptr
    );

    LedStrip(
        ModbusRTUMaster* modbusMaster,
        HALight* haLight,
        const char* name,
        uint8_t pin,
        int ledStripIndexInController,
        uint16_t eepromAddr,
        uint8_t eepromSlots,
        const char* icon = nullptr
    );

    virtual ~LedStrip();

    void setState(bool state);
    void setBrightness(uint8_t brightness);
    void setColorTemperature(uint16_t colorTemperature);
    void setRGBColor(uint8_t r, uint8_t g, uint8_t b);

private:
    void _initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon);
    void _initialize(const char* name, const char* icon);
    void _setup();
    void _saveStateToEeprom();
    bool _initialized = false;

    unsigned long _powerStabilizationStartTime;
    bool _isPowerStabilizing;


    LedStripRegisterSet _register;

    // --- Linked List for Instance Management ---
    LedStrip* _nextInstance;
    static LedStrip* _head;

    // --- Static callback handling ---
    static LedStrip* _findInstance(HALight* haLight);
    static void _onStateCommand(bool state, HALight* sender);
    static void _onBrightnessCommand(uint8_t brightness, HALight* sender);
    static void _onColorTemperatureCommand(uint16_t colorTemperature, HALight* sender);
    static void _onRGBColorCommand(HALight::RGBColor color, HALight* sender);

    static float _miredsToKelvin(int mireds);
    static int _kelvinToMireds(float kelvin);

    void _sendCurrentLedStripDataToController();
    int _getMireds() const;

    int getStartAddress() const;
    int getRegisterAddress(LedStripRegister reg) const;

    inline static int _modbusId = 1;

    ModbusRTUMaster* _modbusMaster;
    HALight* _haLight;
    uint8_t _pin;
    int _ledStripIndexInController;

    // Buffers for name and icon
    char* _haNameBuffer;
    char* _haIconBuffer;

    // EEPROM config
    uint16_t _eepromAddr;
    uint8_t _eepromSlots;

    void _loop();
};


#endif //LEDSTRIP_H
