#include "ControllerRunner.h"

#ifdef DEBUGSTACK
// Define the global semaphore for thread-safe printing, used by TaskMonitor.
SemaphoreHandle_t g_serialSemaphore;
#endif

/**
 * @brief A generic task function (wrapper) used by FreeRTOS to run all configured tasks.
 * @param pvParameters A void pointer to a TaskConfig struct for this specific task.
 */
static void generic_task_wrapper(void* pvParameters)
{
    // Cast the parameter back to a pointer to its TaskConfig struct.
    const TaskConfig* config = static_cast<const TaskConfig*>(pvParameters);

    // 1. Run the one-time setup function if it exists.
    if (config->setupFunc)
    {
        config->setupFunc();
    }

    // Prepare for the periodic delay loop if needed.
    TickType_t xLastWakeTime;
    if (config->delayType == DelayType::PERIODIC)
    {
        xLastWakeTime = xTaskGetTickCount();
    }

    // 2. Enter the infinite task loop.
    for (;;)
    {
        // Choose the appropriate delay type based on the configuration.
        if (config->delayType == DelayType::PERIODIC)
        {
            vTaskDelayUntil(&xLastWakeTime, config->delayTicks);
        }
        else
        {
            // DelayType::SIMPLE
            vTaskDelay(config->delayTicks);
        }

#ifdef DEBUGSTACK
        // Begin measurement if a monitor is assigned.
        if(config->monitor) config->monitor->beginMeasurement();
#endif

        // Execute the main loop function if it exists.
        if (config->loopFunc)
        {
            config->loopFunc();
        }

#ifdef DEBUGSTACK
        // End measurement and report if a monitor is assigned.
        if(config->monitor) {
            config->monitor->endMeasurement();
            config->monitor->report();
        }
#endif
    }
}

void ControllerRunner::run(const TaskConfig tasks[], size_t taskCount)
{
#if defined(DEBUG) || defined(DEBUGSTACK)
    Serial.begin(9600);
#endif

#ifdef DEBUGSTACK
    // Create the mutex semaphore before starting the scheduler.
    g_serialSemaphore = xSemaphoreCreateMutex();
#endif

    DPRINTLN(F("ControllerRunner: Creating tasks..."));

    // Iterate through all provided task configurations.
    for (size_t i = 0; i < taskCount; ++i)
    {
        DPRINT(F("Creating task: "));
        DPRINTLN(tasks[i].name);

        TaskHandle_t taskHandle = NULL; // Handle to the created task.

        xTaskCreate(
            generic_task_wrapper,
            tasks[i].name,
            tasks[i].stackSize,
            (void*)&tasks[i], // Pass a pointer to this task's specific configuration.
            tasks[i].priority,
            &taskHandle // Capture the handle of the created task.
        );

#ifdef DEBUGSTACK
        // If a monitor exists for this task, assign its handle.
        if (tasks[i].monitor != nullptr && taskHandle != NULL) {
            tasks[i].monitor->setTaskHandle(taskHandle);
        }
#endif
    }

    DPRINTLN(F("Starting FreeRTOS scheduler..."));
    vTaskStartScheduler();
}
