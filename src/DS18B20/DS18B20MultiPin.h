#ifndef AHA_DEVICES_DS18B20MULTIPIN_H
#define AHA_DEVICES_DS18B20MULTIPIN_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * @class DS18B20MultiPin
 * @brief Nieblokujący menedżer wielu czujników DS18B20, każdy na osobnym pinie.
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
    // Maszyna stanów
    enum class State : uint8_t
    {
        Idle,
        Requesting,
        Waiting,
        Reading
    };

    // Stałe konfiguracyjne
    static const uint16_t CONVERSION_TIME_MS = 750;     // Czas dla 12-bit
    static const uint32_t MEASUREMENT_INTERVAL_MS = 5000; // Co 5 sekund
    static const uint8_t WINDOW_SIZE = 30;              // Rozmiar okna średniej
    static const uint8_t MAX_ERRORS = 10;                // Ile błędów przed uznaniem za disconnected

    const uint8_t* _pins;
    uint8_t _sensorsCount;

    // Zarządzanie pamięcią i sprzętem
    OneWire** _oneWires;         // Tablica wskaźników do OneWire (1 na pin)
    DallasTemperature* _dallas;  // Współdzielony obiekt Dallas (oszczędność RAM)
    
    // Dane pomiarowe
    float* _lastTemperatures;    // Wynik końcowy (średnia)
    float* _tempSum;             // Suma do obliczania średniej
    uint8_t* _consecutiveErrors; // Licznik błędów z rzędu (do Hot-Swap)
    DeviceAddress* _addresses;   // Adresy czujników

    State _state;
    bool _beginCalled;

    uint32_t _lastRequestTime;
    uint32_t _lastMeasurementTime;

    void _requestAll();
    void _readAll();
    void _searchForMissingSensors();

    // Sprawdza czy adres jest prawidłowy (DS18B20 zaczyna się od 0x28)
    bool _isValidAddress(const DeviceAddress& address);
};

#endif //AHA_DEVICES_DS18B20MULTIPIN_H