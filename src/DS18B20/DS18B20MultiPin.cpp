#include "DS18B20MultiPin.h"
#include "Debug.h"

DS18B20MultiPin::DS18B20MultiPin(const uint8_t* pins, uint8_t sensorsCount)
    : _pins(pins)
      , _sensorsCount(sensorsCount)
      , _oneWires(nullptr)
      , _dallas(nullptr)
      , _lastTemperatures(nullptr)
      , _tempSum(nullptr)
      , _consecutiveErrors(nullptr)
      , _addresses(nullptr)
      , _state(State::Idle)
      , _beginCalled(false)
      , _lastRequestTime(0)
      , _lastMeasurementTime(0)
{
    // Alokacja pamięci
    _oneWires = new OneWire*[_sensorsCount];
    _lastTemperatures = new float[_sensorsCount];
    _tempSum = new float[_sensorsCount];
    _consecutiveErrors = new uint8_t[_sensorsCount]; // Nowa tablica błędów
    _addresses = new DeviceAddress[_sensorsCount];

    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        _oneWires[i] = new OneWire(_pins[i]);
        _lastTemperatures[i] = -127.0f; // Wartość błędu
        _tempSum[i] = -100.0f; // Flaga: brak inicjalizacji średniej
        _consecutiveErrors[i] = 0; // Start z czystym licznikiem błędów

        // Zeruj adresy na starcie
        for (uint8_t j = 0; j < 8; j++) _addresses[i][j] = 0;
    }

    // Jeden obiekt DallasTemperature obsługuje wszystkie magistrale po kolei
    if (_sensorsCount > 0)
    {
        _dallas = new DallasTemperature(_oneWires[0]);
    }
}

DS18B20MultiPin::~DS18B20MultiPin()
{
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
    delete[] _consecutiveErrors;
    delete[] _addresses;
    delete _dallas;
}

void DS18B20MultiPin::begin()
{
    if (_beginCalled || _sensorsCount == 0 || _dallas == nullptr)
    {
        return;
    }

    _dallas->setWaitForConversion(false);

    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // 1. Przełącz magistralę
        _dallas->setOneWire(_oneWires[i]);

        // 2. Skanuj magistralę
        _dallas->begin();

        // 3. Pobierz adres pierwszego urządzenia
        if (_dallas->getAddress(_addresses[i], 0))
        {
            DPRINTLN(F("[DS18B20MultiPin] Sensor found. Setting 12-bit resolution..."));
            _dallas->setResolution(_addresses[i], 12);
        }
        else
        {
            DPRINTLN(F("[DS18B20MultiPin] WARNING: Sensor NOT found on startup!"));
        }
    }

    _beginCalled = true;
    _state = State::Idle;
    _lastMeasurementTime = 0;
    DPRINTLN(F("[DS18B20MultiPin] Manager initialized."));
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
            _searchForMissingSensors(); // Próba znalezienia podłączonych w locie
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
        return -127.0f;
    }
    return _lastTemperatures[pinIndex];
}

bool DS18B20MultiPin::_isValidAddress(const DeviceAddress& address)
{
    // 0x28 to rodzina DS18B20. Jeśli pierwszy bajt to 0, adres jest pusty.
    return address[0] == 0x28;
}

void DS18B20MultiPin::_searchForMissingSensors()
{
    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // Szukamy tylko tam, gdzie adres jest nieprawidłowy (brak czujnika)
        if (!_isValidAddress(_addresses[i]))
        {
            // Przełącz magistralę i szukaj
            _dallas->setOneWire(_oneWires[i]);
            _dallas->begin();

            if (_dallas->getAddress(_addresses[i], 0))
            {
                DPRINT(F("[DS18B20MultiPin] HOT-SWAP: Sensor FOUND on pin "));
                DPRINTLN(i);
                _dallas->setResolution(_addresses[i], 12);

                // Reset filtrów dla nowego czujnika
                _tempSum[i] = -100.0f;
                _consecutiveErrors[i] = 0;
            }
        }
    }
}

void DS18B20MultiPin::_requestAll()
{
    DPRINTLN(F("[DS18B20MultiPin] Requesting conversion..."));
    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        if (_isValidAddress(_addresses[i]))
        {
            _dallas->setOneWire(_oneWires[i]);
            // Adresowanie konkretnego czujnika jest szybsze niż skip()
            _dallas->requestTemperaturesByAddress(_addresses[i]);
        }
    }
}

void DS18B20MultiPin::_readAll()
{
    DPRINTLN(F("[DS18B20MultiPin] Reading temperatures..."));

    for (uint8_t i = 0; i < _sensorsCount; i++)
    {
        // 1. Jeśli nie mamy adresu, pomijamy (czeka na _searchForMissingSensors)
        if (!_isValidAddress(_addresses[i]))
        {
            continue;
        }

        _dallas->setOneWire(_oneWires[i]);
        float tempC = _dallas->getTempC(_addresses[i]);

        // 2. Obsługa błędów i odłączenia (Hot-Swap)
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            _consecutiveErrors[i]++;
            DPRINT(F("[DS18B20MultiPin] Read Error pin "));
            DPRINT(i);
            DPRINT(F(" Count: "));
            DPRINTLN(_consecutiveErrors[i]);

            if (_consecutiveErrors[i] >= MAX_ERRORS)
            {
                DPRINTLN(F(" -> SENSOR DISCONNECTED (Marked invalid)"));
                // Zerujemy adres -> w kolejnym Idle uruchomi się _searchForMissingSensors
                memset(_addresses[i], 0, 8);
                _tempSum[i] = -100.0f; // Reset stanu filtra
                _lastTemperatures[i] = -127.0f; // Sygnał błędu dla usera
                _consecutiveErrors[i] = 0;
            }
            continue; // Pomiń obliczenia średniej
        }

        // 3. Odrzucenie grubych błędów (zakłócenia fizycznie niemożliwe)
        // Margines bezpieczeństwa: -15 do 45
        if (tempC < -15.0f || tempC > 45.0f)
        {
            DPRINTLN(F("[DS18B20MultiPin] Ignored outlier value."));
            continue;
        }

        // Odczyt poprawny -> zerujemy licznik błędów
        _consecutiveErrors[i] = 0;

        // Zaokrąglenie do 2 miejsc po przecinku
        long tX100 = static_cast<long>(tempC * 100.0f + 0.5f);
        float roundedTemp = static_cast<float>(tX100) / 100.0f;

        // 4. Druga weryfikacja (logika aplikacji: -10 do 35)
        if (roundedTemp < -10.0f || roundedTemp > 35.0f)
        {
            continue;
        }

        // 5. Obliczanie średniej (Moving Average)
        if (_tempSum[i] <= -99.0f)
        {
            // FAST START: Pierwszy pomiar ustawia całą średnią
            _tempSum[i] = roundedTemp * static_cast<float>(WINDOW_SIZE);
            _lastTemperatures[i] = roundedTemp;
            DPRINT(F("[DS18B20MultiPin] Initialized pin "));
            DPRINT(i);
            DPRINT(F(" with "));
            DPRINTLN(roundedTemp);
        }
        else
        {
            // Standardowa praca: Odejmij starą średnią, dodaj nowy pomiar
            _tempSum[i] = _tempSum[i] - _lastTemperatures[i] + roundedTemp;
            _lastTemperatures[i] = _tempSum[i] / static_cast<float>(WINDOW_SIZE);

            DPRINT(F("[DS18B20MultiPin] Updated pin "));
            DPRINT(i);
            DPRINT(F(" avg: "));
            DPRINTLN(_lastTemperatures[i]);
        }
    }
}
