#ifndef CONFIG_H
#define CONFIG_H

// -- PINOUT --
#define VOLATGE_SENSOR_PIN 34
#define CURRENT_SENSOR_PIN 35
#define DHT_SENSOR_PIN 4
#define RELAY_PIN 13

// -- WIFI --
#define WIFI_SSID ""
#define WIFI_PASSPHRASE ""
#define WEB_PORT_SERVER 80

// -- SENSOR SETTINGS --
#define ADC_MAX 4095.0
#define SAMPLES 1000
#define VREF 3.3
#define FREQUENCY 60

// -- ZMPT101B (VOLTAGE SENSOR) --
#define VOLTAGE_SENSITIVITY 0.0125
#define VOLTAGE_CALIBRATION 1.0

// ACS712 (CURRENT SENSOR)--
#define CURRENT_SENSITIVITY 0.100
#define CURRENT_CALIBRATION 1.0
#define ACS_OFFSET (VREF/2.0)

// -- TASKS SETTINGS --
#define TASK_ID_WIFI (1 << 0)
#define TASK_ID_SERVER (1 << 1)
#define TASK_ID_CONTROL (1 << 2)
#define TASK_ID_VOLTAGE (1 << 3)
#define TASK_ID_CURRENT (1 << 4)
#define TASK_ID_TEMP (1 << 5)
#define TASK_ID_ANEEL (1 << 6)
#define TASK_ID_COST (1 << 7)

#define ALL_TASKS_ID (TASK_ID_WIFI | TASK_ID_SERVER | TASK_ID_CONTROL | TASK_ID_VOLTAGE | TASK_ID_CURRENT | TASK_ID_TEMP | TASK_ID_ANEEL | TASK_ID_COST)

// ANEEL API --
#define ANEEL_API_URL "https://dadosabertos.aneel.gov.br/api/3/action/datastore_search?resource_id=0591b8f6-fe54-437b-b72b-1aa2efd46e42&limit=1"

// -- COST CALCULATION --
#define BASE_TARIFF 0.75
#endif