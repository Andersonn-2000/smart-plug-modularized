#include "headers/voltage_sensor.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t xVoltageDataQueue;

VoltageData_t readVoltage(){
    VoltageData_t data;
    float sumSq = 0.0f;
    float maxVal = -10.0f;
    float minVal = 10.0f;

    float sampleIntervalMicros = (1000000.0f / FREQUENCY) / (float)SAMPLES;

    for (int i = 0; i < SAMPLES; i++){
        int adc = analogRead(VOLTAGE_SENSOR_PIN);
        float sensorVoltage = ((float)adc / ADC_MAX) * VREF;
        float centered = sensorVoltage - (VREF/2.0f);

        if (centered > maxVal) maxVal = centered;
        if (centered < minVal) minVal = centered;

        sumSq += centered * centered;
        delayMicroseconds((int)sampleIntervalMicros);
    }

    float vpp_sensor = maxVal - minVal;
    float vpeak_sensor = vpp_sensor / 2.0f;
    float vrms_sensor = vpeak_sensor / sqrtf(2.0f);

    float measuredVrms = (vrms_sensor / VOLTAGE_SENSITIVITY) * VOLTAGE_CALIBRATION;
    float measuredVpeak = (vpeak_sensor / VOLTAGE_SENSITIVITY) * VOLTAGE_CALIBRATION;

    data.voltageRMS = measuredVrms;
    data.voltagePeak = measuredVpeak;

    data.voltageOK = (data.voltageRMS > 90.0f && data.voltageRMS < 130.0f) ||
                     (data.voltageRMS > 198.0f && data.voltageRMS < 242.0f);

    return data;
}

void vTaskVoltageMonitoring(void* pvParameters) {
    VoltageData_t voltageData;

    analogReadResolution(12);
    analogSetPinAttenuation(VOLTAGE_SENSOR_PIN, ADC_11db);

    Serial.println("Voltage monitoring task started.");

    while (true) {
        voltageData = readVoltage();
        xQueueOverwrite(xVoltageDataQueue, &voltageData);

        static unsigned long lastLog = 0;
        if (millis() - lastLog > 5000) {
            Serial.printf("Voltage RMS: %.1fV, Peak: %.1fV, Status: %s\n",
                          voltageData.voltageRMS,
                          voltageData.voltagePeak,
                          voltageData.voltageOK ? "OK" : "OUT OF RANGE");
            lastLog = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}