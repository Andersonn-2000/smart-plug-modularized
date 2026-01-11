#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H
#include <Arduino.h>

// -- Enum for relay commands --
typedef enum {
    COMMAND_OFF,
    COMMAND_ON
} RelayCommand_t;

// -- Struct for voltage data --
typedef struct {
    float voltageRMS;
    float voltagePeak;
    bool voltageOK;
} VoltageData_t;

// -- Struct for current data --
typedef struct {
    float currentRMS;
    float currentPeak;
    bool currentOK;
} CurrentData_t;

// -- Struct for temperature and humidity data --
typedef struct {
    float temperature;
    float humidity;
    bool tempOK;
} TemperatureData_t;

// -- STRUCT for ANEEL tariff data --
typedef struct {
    String flagName;
    float flagValue;
    bool tariffOK;
} TariffData_t;

// -- struct for cost data --
typedef struct {
    float power;
    float energy;
    float cost;
    float costOK;
} CostData_t;

#endif