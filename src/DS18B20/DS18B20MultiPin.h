//
// Created by AI Assistant
//

#ifndef AHA_DEVICES_DS18B20MULTIPIN_H
#define AHA_DEVICES_DS18B20MULTIPIN_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * @class DS18B20MultiPin
 * @brief Non-blocking manager for multiple DS18B20 sensors, each on a separate OneWire pin.
 *
 * This class manages multiple DS18B20 temperature sensors where each sensor is connected
 * to its own dedicated pin (one sensor per pin).
 *
 * Wersja ultra-oszczędna pamięciowo:
 * - Używa tablicy obiektów OneWire (1 na pin)
 * - Używa JEDNEGO obiektu DallasTemperature do komunikacji
 * - Przechowuje adresy czujników w wewnętrznej tablicy
 */
class DS18B20MultiPin
{
public:
    DS18B20MultiPin(const uint8_t* pins, uint8_t sensorsCount);
    ~DS18B20MultiPin();

    void begin();
    void loop();
    float getTemperature(uint8_t pinIndex) const;
    uint8_t getSensorCount() const { return _sensorsCount; }

private:
    // Finite state machine
    enum class State : uint8_t
    {
        Idle,
        Requesting,
        Waiting,
        Reading
    };

    // Constants
    static const uint16_t CONVERSION_TIME_MS = 750; // DS18B20 12-bit
    static const uint32_t MEASUREMENT_INTERVAL_MS = 5000; // 30 sekund

    const uint8_t* _pins;
    uint8_t _sensorsCount;

    OneWire** _oneWires; // Tablica wskaźników do OneWire (1 na pin)
    DallasTemperature* _dallas; // Wskaźnik do JEDNEGO obiektu Dallas (oszczędność RAM)
    float* _lastTemperatures; // Ostatnie odczytane temperatury
    DeviceAddress* _addresses; // ***NOWOŚĆ: Tablica na adresy czujników***

    State _state;
    bool _beginCalled;

    uint32_t _lastRequestTime;
    uint32_t _lastMeasurementTime;

    void _requestAll();
    void _readAll();

    // Funkcja pomocnicza do sprawdzania, czy adres jest poprawny (nie same zera)
    bool _isValidAddress(const DeviceAddress& address);
};

#endif //AHA_DEVICES_DS18B20MULTIPIN_H