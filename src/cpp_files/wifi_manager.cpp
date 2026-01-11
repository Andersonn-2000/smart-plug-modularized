#include "headers/wifi_manager.h"
#include "headers/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <WiFi.h>

extern EventGroupHandle_t xWatchdogGroupHandle;

void vTaskWifiConnect(void* pvParameters) {
    Serial.println("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
        xEventGroupSetBits(xWatchdogGroupHandle, TASK_ID_WIFI);
    } else {
        Serial.println("\nFailed to connect to WiFi.");
    }

    vTaskDelete(NULL);
}