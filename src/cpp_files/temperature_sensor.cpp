#include "headers/temperature_sensor.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <DHTesp.h>

extern QueueHandle_t xTemperatureDataQueue;
DHTesp dht;

TemperatureData_t readTemperature() {
    TemperatureData_t data;
    TempAndHumidity newValues = dht.getTempAndHumidity();

    if (dht.getStatus() == 0) {
        data.temperature = newValues.temperature;
        data.humidity = newValues.humidity;
        data.tempOK = true;
    } else {
        data.temperature = 0.0f;
        data.humidity = 0.0f;
        data.tempOK = false;
    }

    return data;
}

void vTaskTemperatureMonitoring(void* pvParameters) {
    dht.setup(DHT_SENSOR_PIN, DHTesp::DHT11);
    Serial.println("Temperature monitoring task started.");

    while (true) {
        TemperatureData_t tempData = readTemperature();
        xQueueOverwrite(xTemperatureDataQueue, &tempData);

        static unsigned long lastLog = 0;
        if (millis() - lastLog > 5000) {
            if(tempData.tempOK) {
              Serial.printf("Temperature: %.1f°C, Humidity: %.1f%%\n", tempData.temperature, tempData.humidity);
            } else {
              Serial.println("Failed to read from DHT sensor.");
            }
            lastLog = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // DHT11 has a 2-second sample rate
    }
}
