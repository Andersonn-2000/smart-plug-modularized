#include "headers/aneel_api.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

extern QueueHandle_t xTariffDataQueue;

void vTaskTariffUpdate(void* pvParameters) {
    Serial.println("ANEEL API task started.");

    while (true) {
        HTTPClient http;
        TariffData_t tariffData;

        http.begin(ANEEL_API_URL);
        int httpCode = http.GET();

        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                DynamicJsonDocument doc(1024);
                deserializeJson(doc, payload);

                JsonObject result = doc["result"];
                JsonArray records = result["records"];
                JsonObject firstRecord = records[0];

                tariffData.flagName = String(firstRecord["NomBandeiraAcionada"].as<char*>());
                tariffData.flagValue = firstRecord["VlrAdicionalBandeira"].as<float>();
                tariffData.tariffOK = true;

                xQueueOverwrite(xTariffDataQueue, &tariffData);
                Serial.printf("ANEEL Tariff Updated: %s - R$%.2f\n", tariffData.flagName.c_str(), tariffData.flagValue);
            } else {
                tariffData.tariffOK = false;
                 xQueueOverwrite(xTariffDataQueue, &tariffData);
                Serial.printf("Failed to get ANEEL data, http code: %d\n", httpCode);
            }
        } else {
            tariffData.tariffOK = false;
            xQueueOverwrite(xTariffDataQueue, &tariffData);
            Serial.printf("Failed to connect to ANEEL API, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();

        // Update once a day
        vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 60 * 24));
    }
}