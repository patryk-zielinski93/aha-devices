#include "DS18B20MultiPin.h"
#include "Debug.h"

DS18B20MultiPin::DS18B20MultiPin(const uint8_t* pins, uint8_t sensorsCount)
    : _pins(pins)
      , _sensorsCount(sensorsCount)
      , _oneWires(nullptr)
      , _dallas(nullptr)
      , _lastTemperatures(nullptr)
      , _tempSum(nullptr)
      , _readsCount(nullptr)
      , _addresses(nullptr)
      , _state(State::Idle)
      , _beginCalled(false)
      , _lastRequestTime(0)
      , _lastMeasurementTime(0)
      , _totalReadAttempts(0)
{
    // Alokacja tablicy na obiekty OneWire
    _oneWires = new OneWire*[_sensorsCount];
    // Alokacja tablicy na ostatnie obliczone średnie (zwracane przez getTemperature)
    _lastTemperatures = new float[_sensorsCount];
    // Alokacja tablicy na sumy temperatur (bieżący cykl 50 pomiarów)
    _tempSum = new float[_sensorsCount];
    // Alokacja tablicy na liczniki udanych odczytów
    _readsCount = new uint8_t[_sensorsCount];
    // Alokacja tablicy na adresy (8 bajtów na czujnik)
    _addresses = new DeviceAddress[_sensorsCount];

    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        _oneWires[i] = new OneWire(_pins[i]);
        _lastTemperatures[i] = -127.0f; // Domyślny "disconnected"
        _tempSum[i] = 0.0f; // Suma = 0 na starcie
        _readsCount[i] = 0; // Brak odczytów na starcie
        // Zeruj adresy na starcie
        for (uint8_t j = 0; j < 8; j++) _addresses[i][j] = 0;
    }

    // Alokuj JEDEN obiekt DallasTemperature
    if (_sensorsCount > 0)
    {
        _dallas = new DallasTemperature(_oneWires[0]);
    }
}

DS18B20MultiPin::~DS18B20MultiPin()
{
    // Czyszczenie pamięci
    if (_oneWires)
    {
        for (uint8_t i = 0; i < _sensorsCount; i++)
        {
            delete _oneWires[i];
        }
        delete[] _oneWires;
    }
    delete[] _lastTemperatures;
    delete[] _tempSum;
    delete[] _readsCount;
    delete[] _addresses;
    delete _dallas;
}

void DS18B20MultiPin::begin()
{
    if (_beginCalled || _sensorsCount == 0 || _dallas == nullptr)
    {
        return;
    }

    // Ustaw tryb nieblokujący RAZ
    _dallas->setWaitForConversion(false);

    // ***ZMIANA: Pętla inicjalizująca KAŻDY pin OSOBNO***
    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // 1. Przełącz magistralę
        _dallas->setOneWire(_oneWires[i]);

        // 2. Wywołaj begin() (skanuje TĘ magistralę)
        _dallas->begin();

        // 3. Pobierz adres JEDYNEGO czujnika (indeks 0) na tej magistrali
        if (_dallas->getAddress(_addresses[i], 0))
        {
            DPRINTLN(F("[DS18B20MultiPin] Sensor found. Setting 12-bit resolution..."));

            // 4. Ustaw rozdzielczość używając ADRESU (zapis do EEPROM czujnika)
            _dallas->setResolution(_addresses[i], 12);
        }
        else
        {
            DPRINTLN(F("[DS18B20MultiPin] ERROR! Sensor NOT found!"));
            // Adres pozostanie wyzerowany
        }
    }

    _beginCalled = true;
    _state = State::Idle;
    _lastMeasurementTime = 0;
    DPRINTLN(F("[DS18B20MultiPin] All sensors initialized."));
}

void DS18B20MultiPin::loop()
{
    if (!_beginCalled || _sensorsCount == 0 || _dallas == nullptr)
    {
        return;
    }

    uint32_t now = millis();

    switch (_state)
    {
    case State::Idle:
        if ((now - _lastMeasurementTime) >= MEASUREMENT_INTERVAL_MS || _lastMeasurementTime == 0)
        {
            _searchForMissingSensors();
            _requestAll();
            _lastRequestTime = now;
            _state = State::Requesting;
        }
        break;

    case State::Requesting:
        _state = State::Waiting;
        break;

    case State::Waiting:
        if ((now - _lastRequestTime) >= CONVERSION_TIME_MS)
        {
            _state = State::Reading;
        }
        break;

    case State::Reading:
        _readAll();
        _lastMeasurementTime = now;
        _state = State::Idle;
        break;
    }
}

float DS18B20MultiPin::getTemperature(uint8_t pinIndex) const
{
    if (pinIndex >= _sensorsCount)
    {
        return -127.0f; // invalid index
    }

    // Zwróć ostatnią obliczoną średnią (aktualizowaną co 50 pomiarów)
    return _lastTemperatures[pinIndex];
}

/**
 * @brief Sprawdza, czy adres nie jest pusty (np. 0x00...)
 */
bool DS18B20MultiPin::_isValidAddress(const DeviceAddress& address)
{
    // Adres DS18B20 zawsze zaczyna się od 0x28
    return address[0] == 0x28;
}

void DS18B20MultiPin::_searchForMissingSensors()
{
    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // Szukaj tylko tam, gdzie czujnik nie został jeszcze odnaleziony
        if (!_isValidAddress(_addresses[i]))
        {
            DPRINT(F("[DS18B20MultiPin] Searching on pin "));
            DPRINT(i);
            DPRINTLN(F("..."));

            // Przełącz magistralę
            _dallas->setOneWire(_oneWires[i]);

            // Wywołaj begin() ponownie
            _dallas->begin();

            // Spróbuj pobrać adres
            if (_dallas->getAddress(_addresses[i], 0))
            {
                DPRINT(F("[DS18B20MultiPin] Sensor FOUND on pin "));
                DPRINTLN(i);

                // Ustaw rozdzielczość
                _dallas->setResolution(_addresses[i], 12);
            }
        }
    }
}

void DS18B20MultiPin::_requestAll()
{
    DPRINTLN(F("[DS18B20MultiPin] Requesting temperature conversion..."));
    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // Kontynuuj tylko jeśli mamy poprawny adres dla tego pinu
        if (_isValidAddress(_addresses[i]))
        {
            _dallas->setOneWire(_oneWires[i]);
            // ***ZMIANA: Żądaj konwersji używając ADRESU***
            _dallas->requestTemperaturesByAddress(_addresses[i]);
        }
    }
}

void DS18B20MultiPin::_readAll()
{
    DPRINTLN(F("[DS18B20MultiPin] Reading temperatures..."));

    // Inkrementuj globalny licznik prób odczytu
    _totalReadAttempts++;

    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // Odczytuj tylko jeśli mamy poprawny adres
        if (!_isValidAddress(_addresses[i]))
        {
            // Ten czujnik nie został znaleziony podczas begin()
            DPRINTLN(F("[DS18B20MultiPin] Sensor not found (invalid address)"));
            continue; // Pomijamy ten pomiar, nie dodajemy do sumy
        }

        _dallas->setOneWire(_oneWires[i]);
        // Pobierz temperaturę używając ADRESU
        float tempC = _dallas->getTempC(_addresses[i]);

        if (tempC == DEVICE_DISCONNECTED_C)
        {
            // Czujnik rozłączony - wyzeruj adres i pomiń pomiar
            for (uint8_t j = 0; j < 8; j++) _addresses[i][j] = 0;
            DPRINTLN(F("[DS18B20MultiPin] Sensor disconnected"));
            continue; // Pomijamy ten pomiar, nie dodajemy do sumy
        }
        else
        {
            // Pomiar udany - dodaj do sumy i inkrementuj licznik
            long tX100 = (long)(tempC * 100.0f + 0.5f);
            float roundedTemp = (float)tX100 / 100.0f;

            if (roundedTemp < -10.0f || roundedTemp > 42.0f)
            {
                // Odczyt mało prawdopodobny, pomiń
                continue;
            }
            _tempSum[i] += roundedTemp; // Dodaj do sumy
            _readsCount[i]++; // Inkrementuj licznik udanych odczytów

            DPRINT(F("[DS18B20MultiPin] Read success: "));
            DPRINT(roundedTemp);
            DPRINT(F("°C (reads in cycle: "));
            DPRINT(_readsCount[i]);
            DPRINTLN(F(")"));
        }
    }

    // Jeżeli osiągnięto 50 prób, oblicz średnie i resetuj liczniki
    if (_totalReadAttempts >= MAX_READINGS)
    {
        DPRINTLN(F("[DS18B20MultiPin] *** Completing 50-read cycle, updating averages ***"));

        for (uint8_t i = 0; i < _sensorsCount; i++)
        {
            if (_readsCount[i] > 0)
            {
                // Oblicz średnią i zapisz jako wartość zwracaną
                _lastTemperatures[i] = _tempSum[i] / _readsCount[i];

                DPRINT(F("[DS18B20MultiPin] Sensor "));
                DPRINT(i);
                DPRINT(F(": Average = "));
                DPRINT(_lastTemperatures[i]);
                DPRINT(F("°C (from "));
                DPRINT(_readsCount[i]);
                DPRINTLN(F(" valid reads)"));
            }
            else
            {
                // Brak udanych odczytów w cyklu - ustaw -127
                _lastTemperatures[i] = -127.0f;

                DPRINT(F("[DS18B20MultiPin] Sensor "));
                DPRINT(i);
                DPRINTLN(F(": No valid reads, returning -127"));
            }

            // Resetuj liczniki dla następnego cyklu
            _tempSum[i] = 0.0f;
            _readsCount[i] = 0;
        }

        // Resetuj globalny licznik prób
        _totalReadAttempts = 0;
    }
}
