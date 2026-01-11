#ifndef VOLTAGE_SENSOR_H
#define VOLATGE_SENSOR_H
#include "data_structures.h"

void vTaskVoltageMonitoring(void* pvParameters);
VoltageData_t readVoltage();

#endif