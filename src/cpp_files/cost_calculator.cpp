#include "headers/cost_calculator.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t xVoltageDataQueue;
extern QueueHandle_t xCurrentDataQueue;
extern QueueHandle_t xTariffDataQueue;
extern QueueHandle_t xCostDataQueue;

void vTaskCostCalculation(void* pvParameters) {
    VoltageData_t voltageData;
    CurrentData_t currentData;
    TariffData_t tariffData;
    CostData_t costData;

    float totalEnergy = 0;
    unsigned long lastTime = millis();

    Serial.println("Cost calculation task started.");

    while (true) {
        if (xQueueReceive(xVoltageDataQueue, &voltageData, 0) != pdTRUE) {
            voltageData.voltageRMS = 0;
        }
        if (xQueueReceive(xCurrentDataQueue, &currentData, 0) != pdTRUE) {
            currentData.currentRMS = 0;
        }
        if (xQueueReceive(xTariffDataQueue, &tariffData, 0) != pdTRUE) {
            tariffData.flagValue = 0;
        }

        unsigned long currentTime = millis();
        float elapsedTime = (currentTime - lastTime) / 3600000.0; // in hours
        lastTime = currentTime;

        costData.power = voltageData.voltageRMS * currentData.currentRMS;
        totalEnergy += (costData.power * elapsedTime) / 1000.0; // in kWh
        costData.energy = totalEnergy;

        float baseCost = totalEnergy * BASE_TARIFF;
        float flagCost = (totalEnergy / 100.0) * tariffData.flagValue;
        costData.cost = baseCost + flagCost;
        costData.costOK = true;

        xQueueOverwrite(xCostDataQueue, &costData);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}