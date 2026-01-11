#include "headers/relay_control.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

extern QueueHandle_t xRelayCommandQueue;
extern SemaphoreHandle_t xRelayStateMutex;
extern volatile int relayState;

void vTaskRelayControl(void* pvParameters) {
    RelayCommand_t command;
    int currentState = LOW;

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, currentState);

    xSemaphoreTake(xRelayStateMutex, portMAX_DELAY);
    relayState = currentState;
    xSemaphoreGive(xRelayStateMutex);

    Serial.printf("Relay (Pin %d) initialized in LOW (OFF)\n", RELAY_PIN);

    while (true) {
        if (xQueueReceive(xRelayCommandQueue, &command, portMAX_DELAY) == pdTRUE) {
            int newState = (command == COMMAND_ON) ? HIGH : LOW;

            if (newState != currentState) {
                digitalWrite(RELAY_PIN, newState);
                currentState = newState;

                xSemaphoreTake(xRelayStateMutex, portMAX_DELAY);
                relayState = currentState;
                xSemaphoreGive(xRelayStateMutex);

                Serial.printf("-> NEW COMMAND EXECUTED: Relay changed to %s\n",
                              (currentState == HIGH ? "ON" : "OFF"));
            } else {
                Serial.println("Command received same as current state (no change).");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
