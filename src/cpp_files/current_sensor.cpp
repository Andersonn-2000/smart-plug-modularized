#include "headers/current_sensor.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t xCurrentDataQueue;

CurrentData_t readCurrent() {
    CurrentData_t data;
    float sumSq = 0.0f;
    float maxVal = -10.0f;
    float minVal = 10.0f;

    float sampleIntervalMicros = (1000000.0f / FREQUENCY) / (float)SAMPLES;

    for (int i = 0; i < SAMPLES; i++) {
        int adc = analogRead(CURRENT_SENSOR_PIN);
        float sensorVoltage = ((float)adc / ADC_MAX) * VREF;
        float centered = sensorVoltage - ACS_OFFSET;

        if (centered > maxVal) maxVal = centered;
        if (centered < minVal) minVal = centered;

        sumSq += centered * centered;
        delayMicroseconds((int)sampleIntervalMicros);
    }

    float vpp_sensor = maxVal - minVal;
    float vpeak_sensor = vpp_sensor / 2.0f;
    float vrms_sensor = vpeak_sensor / sqrtf(2.0f);

    float measuredIrms = (vrms_sensor / CURRENT_SENSITIVITY) * CURRENT_CALIBRATION;
    float measuredIpeak = (vpeak_sensor / CURRENT_SENSITIVITY) * CURRENT_CALIBRATION;

    data.currentRMS = measuredIrms;
    data.currentPeak = measuredIpeak;

    data.currentOK = (data.currentRMS < 10.0f);

    return data;
}

void vTaskCurrentMonitoring(void* pvParameters) {
    CurrentData_t currentData;

    analogReadResolution(12);
    analogSetPinAttenuation(CURRENT_SENSOR_PIN, ADC_11db);

    Serial.println("Current monitoring task started.");

    while (true) {
        currentData = readCurrent();
        xQueueOverwrite(xCurrentDataQueue, &currentData);

        static unsigned long lastLog = 0;
        if (millis() - lastLog > 5000) {
            Serial.printf("Current RMS: %.3fA, Peak: %.3fA, Status: %s\n",
                          currentData.currentRMS,
                          currentData.currentPeak,
                          currentData.currentOK ? "OK" : "HIGH");
            lastLog = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
