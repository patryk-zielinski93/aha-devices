#ifndef AHA_DEVICES_CONTROLLER_RUNNER_H
#define AHA_DEVICES_CONTROLLER_RUNNER_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <Debug.h>

// Forward-declare TaskMonitor to avoid circular dependencies if needed,
// but including it directly is fine here.
#ifdef DEBUGSTACK
#include "TaskMonitor.h"
#endif

/**
 * @enum DelayType
 * @brief Specifies the type of delay to use within a task's loop.
 */
enum class DelayType
{
    PERIODIC, // Use vTaskDelayUntil for precise, periodic execution.
    SIMPLE // Use vTaskDelay for a simple, non-blocking delay.
};

/**
 * @struct TaskConfig
 * @brief Describes a single, complete RTOS task.
 */
struct TaskConfig
{
    const char* name; // Task name for debugging.
    void (*setupFunc)(); // Pointer to the setup function, runs once.
    void (*loopFunc)(); // Pointer to the loop function, runs repeatedly.
    uint16_t stackSize; // Stack size for the task.
    UBaseType_t priority; // Priority of the task.
    DelayType delayType = DelayType::PERIODIC; // Default delay type.
    TickType_t delayTicks = pdMS_TO_TICKS(20); // Default delay value.

#ifdef DEBUGSTACK
    // Optional pointer to a monitor instance for this task.
    TaskMonitor* monitor = nullptr;
#endif
};

namespace ControllerRunner
{
    /**
     * @brief Initializes and starts the FreeRTOS scheduler with a given set of tasks.
     * @param tasks An array of TaskConfig structs that define all tasks to be created.
     * @param taskCount The number of elements in the tasks array.
     */
    void run(const TaskConfig tasks[], size_t taskCount);
}

#endif // AHA_DEVICES_CONTROLLER_RUNNER_H
