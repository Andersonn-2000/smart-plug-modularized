#include <Arduino.h>
#include "headers/config.h"
#include "headers/data_structures.h"
#include "headers/voltage_sensor.h"
#include "headers/current_sensor.h"
#include "headers/temperature_sensor.h"
#include "headers/cost_calculator.h"
#include "headers/aneel_api.h"
#include "headers/relay_control.h"
#include "headers/web_server.h"
#include "headers/wifi_manager.h"
#include "headers/watchdog.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

// --- Shared Resources ---
QueueHandle_t xRelayCommandQueue;
QueueHandle_t xVoltageDataQueue;
QueueHandle_t xCurrentDataQueue;
QueueHandle_t xTemperatureDataQueue;
QueueHandle_t xTariffDataQueue;
QueueHandle_t xCostDataQueue;

SemaphoreHandle_t xWifiMutex;
SemaphoreHandle_t xRelayStateMutex;
SemaphoreHandle_t xCostDataMutex;

EventGroupHandle_t xWatchdogGroupHandle;

volatile int relayState = LOW;

void setup() {
    Serial.begin(115200);
    delay(100);

    // --- Initialize FreeRTOS Resources ---
    xRelayCommandQueue = xQueueCreate(5, sizeof(RelayCommand_t));
    xVoltageDataQueue = xQueueCreate(1, sizeof(VoltageData_t));
    xCurrentDataQueue = xQueueCreate(1, sizeof(CurrentData_t));
    xTemperatureDataQueue = xQueueCreate(1, sizeof(TemperatureData_t));
    xTariffDataQueue = xQueueCreate(1, sizeof(TariffData_t));
    xCostDataQueue = xQueueCreate(1, sizeof(CostData_t));

    xWifiMutex = xSemaphoreCreateMutex();
    xRelayStateMutex = xSemaphoreCreateMutex();
    xCostDataMutex = xSemaphoreCreateMutex();

    xWatchdogGroupHandle = xEventGroupCreate();

    // --- Create Tasks ---
    xTaskCreate(vTaskWifiConnect, "Wifi Connect", 4096, NULL, 3, NULL);
    xTaskCreate(vTaskRelayControl, "Relay Control", 4096, NULL, 2, NULL);
    xTaskCreate(vTaskVoltageMonitoring, "Voltage Monitor", 8192, NULL, 2, NULL);
    xTaskCreate(vTaskCurrentMonitoring, "Current Monitor", 8192, NULL, 2, NULL);
    xTaskCreate(vTaskTemperatureMonitoring, "Temperature Monitor", 4096, NULL, 2, NULL);
    xTaskCreate(vTaskCostCalculation, "Cost Calculator", 4096, NULL, 2, NULL);
    xTaskCreate(vTaskTariffUpdate, "ANEEL API", 8192, NULL, 1, NULL);
    xTaskCreate(vTaskServer, "Web Server", 8192, NULL, 1, NULL);
    xTaskCreate(vTaskWatchdog, "Task Watchdog", 4096, NULL, 1, NULL);

    Serial.println("System initialized. Waiting for WiFi connection...");
}

void loop() {
    // The loop is not used in a FreeRTOS application
    vTaskDelete(NULL);
}