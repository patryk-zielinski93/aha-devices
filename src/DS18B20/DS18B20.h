//
// Created by zielq on 28.09.2025.
//

#ifndef AHA_DEVICES_DS18B20_H
#define AHA_DEVICES_DS18B20_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * @class DS18B20
 * @brief Non-blocking manager for multiple DS18B20 sensors on a single OneWire bus.
 *
 * This version only updates the temperature array once every MEASUREMENT_INTERVAL_MS.
 * In the internal state machine (loop()), we do:
 *    - IDLE: wait until it's time to start a new measurement (>= 60s),
 *    - REQUESTING: send requestTemperatures(), immediately go WAITING,
 *    - WAITING: after 750ms (conversion time), go READ,
 *    - READING: read all sensors, store them in _lastTemperatures, go IDLE again.
 */
class DS18B20
{
public:
    /**
     * @param pin The Arduino pin number where the OneWire bus is connected.
     * @param deviceAddresses Pointer to an array of DeviceAddress, each containing 8 bytes of sensor address.
     * @param sensorCount Number of sensors in the deviceAddresses array.
     */
    DS18B20(uint8_t pin, const DeviceAddress* deviceAddresses, uint8_t sensorCount);

    /**
     * @brief Initializes the DallasTemperature library. Call this in setup().
     */
    void begin();

    /**
     * @brief Non-blocking loop method to be called frequently in main loop().
     *        The class only reads new temperature results once per minute.
     */
    void loop();

    /**
     * @brief Gets the last measured temperature (in Celsius) for a specific sensor index.
     * @param index Sensor index [0..sensorCount-1].
     * @return Temperature in °C with two decimals, or -127.00 if sensor is disconnected or invalid index.
     */
    float getTemperature(uint8_t index) const;

    /**
     * @brief Returns the total number of sensors managed by this instance.
     */
    uint8_t getSensorCount() const { return _sensorCount; }

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
    static const uint16_t CONVERSION_TIME_MS = 750; // DS18B20 conversion at 12-bit
    static const uint32_t MEASUREMENT_INTERVAL_MS = 60000; // 1 minute interval

    OneWire _oneWire; // OneWire bus object
    DallasTemperature _dallas; // DallasTemperature object
    const DeviceAddress* _deviceAddrs; // Pointer to array of sensor addresses
    uint8_t _sensorCount; // Number of sensors

    float* _lastTemperatures; // Storage for last read temperatures
    State _state; // Current state
    bool _beginCalled; // If begin() was called

    uint32_t _lastRequestTime; // Timestamp of last DS18B20 request
    uint32_t _lastMeasurementTime; // Timestamp of the last completed measurement

    // Internal helper methods
    void _requestAll();
    void _readAll();
};

#endif //AHA_DEVICES_DS18B20_H
