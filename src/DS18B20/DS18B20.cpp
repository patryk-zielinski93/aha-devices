#include "DS18B20.h"

#include "Debug.h"

DS18B20::DS18B20(uint8_t pin, const DeviceAddress* deviceAddresses, uint8_t sensorCount)
    : _oneWire(pin)
      , _dallas(&_oneWire)
      , _deviceAddrs(deviceAddresses)
      , _sensorCount(sensorCount)
      , _lastTemperatures(nullptr)
      , _state(State::Idle)
      , _beginCalled(false)
      , _lastRequestTime(0)
      , _lastMeasurementTime(0)
{
    // Allocate memory for temperature storage
    _lastTemperatures = new float[_sensorCount];
    for (uint8_t i = 0; i < _sensorCount; i++)
    {
        _lastTemperatures[i] = -127.0f; // default "disconnected"
    }
}

void DS18B20::begin()
{
    if (_beginCalled)
    {
        return;
    }

    _dallas.begin();
    // Non-blocking conversion mode
    _dallas.setWaitForConversion(false);

    // Optionally set resolution for each sensor (12 bits)
    for (uint8_t i = 0; i < _sensorCount; i++)
    {
        bool valid = false;
        for (uint8_t b = 0; b < 8; b++)
        {
            if (_deviceAddrs[i][b] != 0)
            {
                valid = true;
                break;
            }
        }
        if (valid)
        {
            _dallas.setResolution((uint8_t*)_deviceAddrs[i], 11, false);
        }
    }
    _dallas.setResolution(11);

    _beginCalled = true;
    _state = State::Idle;
    _lastMeasurementTime = 0;
    DPRINTLN(F("[DS18B20] Initialized."));
}

void DS18B20::loop()
{
    if (!_beginCalled || _sensorCount == 0)
    {
        return;
    }

    uint32_t now = millis();

    switch (_state)
    {
    case State::Idle:
        // Wait until it's time for the next measurement
        if ((now - _lastMeasurementTime) >= MEASUREMENT_INTERVAL_MS || now == 0)
        {
            _requestAll();
            _lastRequestTime = now;
            _state = State::Requesting;
        }
        break;

    case State::Requesting:
        // Immediately after request, we go to WAITING
        _state = State::Waiting;
        break;

    case State::Waiting:
        // Check if enough time (750 ms) passed for the DS18B20 to finish conversion
        if ((now - _lastRequestTime) >= CONVERSION_TIME_MS)
        {
            _state = State::Reading;
        }
        break;

    case State::Reading:
        // Read new temperatures from all sensors
        _readAll();
        // We completed one measurement cycle
        _lastMeasurementTime = now;
        // Move back to Idle
        _state = State::Idle;
        break;
    }
}

float DS18B20::getTemperature(uint8_t index) const
{
    if (index >= _sensorCount)
    {
        return -127.0f; // invalid index
    }
    return _lastTemperatures[index];
}

void DS18B20::_requestAll()
{
    _dallas.requestTemperatures();
}

void DS18B20::_readAll()
{
    for (uint8_t i = 0; i < _sensorCount; i++)
    {
        float tempC = _dallas.getTempC((uint8_t*)_deviceAddrs[i]);
        if (tempC == DEVICE_DISCONNECTED_C)
        {
            _lastTemperatures[i] = -127.0f;
        }
        else
        {
            // Round to two decimals
            long tX100 = (long)(tempC * 100.0f + 0.5f);
            _lastTemperatures[i] = (float)tX100 / 100.0f;
        }
    }
}
