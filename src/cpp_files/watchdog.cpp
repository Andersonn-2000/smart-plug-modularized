#include "headers/watchdog.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t xWatchdogGroupHandle;

void vTaskWatchdog(void* pvParameters) {
    const TickType_t waitTicks = pdMS_TO_TICKS(15000);

    while (true) {
        EventBits_t status = xEventGroupWaitBits(
            xWatchdogGroupHandle,
            ALL_TASKS_ID,
            pdTRUE,  // clear bits on exit
            pdTRUE,  // wait for all bits
            waitTicks
        );

        if ((status & ALL_TASKS_ID) == ALL_TASKS_ID) {
            // All tasks are running
        } else {
            Serial.println("--- WATCHDOG ALERT: A task has failed! ---");
            Serial.printf("Status: 0x%08X\n", (unsigned int)status);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}