#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H
#include "data_structures.h"

void vTaskTemperatureMonitoring(void* pvParameters);
TemperatureData_t readTemperature();

#endif