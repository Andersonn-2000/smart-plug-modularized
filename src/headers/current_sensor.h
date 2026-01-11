#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H
#include "data_structures.h"

void vTaskCurrentMonitoring(void* pvParameters);
CurrentData_t readCurrent();

#endif