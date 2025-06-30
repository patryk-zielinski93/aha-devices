#ifndef TASK_MONITOR_H
#define TASK_MONITOR_H

#include "Debug.h" // This file controls the DEBUG macro

#ifdef DEBUGSTACK // Only compile this entire class if DEBUG is enabled

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

// External declaration of the semaphore handle.
// It tells the compiler that this variable exists and will be defined elsewhere.
extern SemaphoreHandle_t g_serialSemaphore;

/**
 * @brief A smart, thread-safe macro for printing debug information.
 * It automatically detects if the RTOS scheduler is running.
 * - If called from setup(), it prints directly to Serial.
 * - If called from a task, it uses a mutex semaphore for thread safety.
 */
#define SDPRINT(...) do { \
if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) { \
/* RTOS scheduler is running, use semaphore for thread safety */ \
if (g_serialSemaphore != NULL && xSemaphoreTake(g_serialSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) { \
Serial.print(__VA_ARGS__); \
xSemaphoreGive(g_serialSemaphore); \
} \
} else { \
/* RTOS not started (we are in setup()), print directly */ \
Serial.print(__VA_ARGS__); \
} \
} while(0)

#define SDPRINTLN(...) do { \
if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) { \
/* RTOS scheduler is running, use semaphore */ \
if (g_serialSemaphore != NULL && xSemaphoreTake(g_serialSemaphore, pdMS_TO_TICKS(10)) == pdTRUE) { \
Serial.println(__VA_ARGS__); \
xSemaphoreGive(g_serialSemaphore); \
} \
} else { \
/* RTOS not started, print directly */ \
Serial.println(__VA_ARGS__); \
} \
} while(0)


/**
 * @class TaskMonitor
 * @brief A helper class to measure and log execution stats and stack usage for an RTOS task.
 */
class TaskMonitor {
private:
    // --- Configuration ---
    const char* _name;
    TaskHandle_t _taskHandle;
    uint16_t _logInterval_ms;

    // --- Statistics for the measurement interval ---
    unsigned long _executionCount;
    unsigned long _totalExecutionTime_us;
    unsigned long _minExecutionTime_us;
    unsigned long _maxExecutionTime_us;
    
    // --- Internal state ---
    unsigned long _startTime_us;
    unsigned long _lastLogTime_ms;

public:
    TaskMonitor(const char* name, TaskHandle_t taskHandle = NULL, uint16_t logInterval = 15000)
        : _name(name),
          _taskHandle(taskHandle),
          _logInterval_ms(logInterval),
          _executionCount(0),
          _totalExecutionTime_us(0),
          _minExecutionTime_us(0xFFFFFFFF),
          _maxExecutionTime_us(0),
          _startTime_us(0),
          _lastLogTime_ms(0)
    {}

    void setTaskHandle(TaskHandle_t handle) {
        _taskHandle = handle;
    }

    void beginMeasurement() {
        _startTime_us = micros();
    }

    void endMeasurement() {
        unsigned long duration = micros() - _startTime_us;
        _totalExecutionTime_us += duration;
        _executionCount++;
        if (duration < _minExecutionTime_us) _minExecutionTime_us = duration;
        if (duration > _maxExecutionTime_us) _maxExecutionTime_us = duration;
    }

    void report() {
        if (millis() - _lastLogTime_ms >= _logInterval_ms) {
            _lastLogTime_ms = millis();
            
            unsigned long avg = (_executionCount > 0) ? (_totalExecutionTime_us / _executionCount) : 0;
            
            // --- CORRECTED LOGGING ---
            // Now using our thread-safe DPRINT/DPRINTLN macros.
            // No need for manual semaphore handling here.
            SDPRINTLN(F("--- Task Performance Report ---"));
            SDPRINT(F(" Name: ")); DPRINTLN(_name);
            SDPRINT(F(" Executions in interval: ")); DPRINTLN(_executionCount);
            SDPRINT(F(" Time (us)  Min/Avg/Max: "));
            SDPRINT(_minExecutionTime_us); DPRINT(F("/"));
            SDPRINT(avg); DPRINT(F("/"));
            SDPRINTLN(_maxExecutionTime_us);
            
            if (_taskHandle != NULL) {
                UBaseType_t hwm = uxTaskGetStackHighWaterMark(_taskHandle);
                SDPRINT(F(" Stack HWM (free bytes): ")); DPRINTLN(hwm * sizeof(StackType_t));
            }
            SDPRINTLN(F("-------------------------------"));

            // Reset stats for the next interval
            _executionCount = 0;
            _totalExecutionTime_us = 0;
            _minExecutionTime_us = 0xFFFFFFFF;
            _maxExecutionTime_us = 0;
        }
    }
};

#else
// When DEBUG is disabled, macros are empty - zero overhead.
#define SDPRINT(...)
#define SDPRINTLN(...)
#endif

#endif // TASK_MONITOR_H
