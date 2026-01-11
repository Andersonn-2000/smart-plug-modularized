#include "headers/web_server.h"
#include "headers/config.h"
#include "headers/data_structures.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <WebServer.h>

extern QueueHandle_t xRelayCommandQueue;
extern QueueHandle_t xVoltageDataQueue;
extern QueueHandle_t xCurrentDataQueue;
extern QueueHandle_t xTemperatureDataQueue;
extern QueueHandle_t xCostDataQueue;
extern SemaphoreHandle_t xRelayStateMutex;
extern volatile int relayState;

WebServer server(WEB_PORT_SERVER);

const char* HTML_CONTENT = R"raw(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Plug ESP32</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            background-color: #f4f4f9;
            text-align: center;
            padding: 20px;
        }
        .container {
            background-color: #ffffff;
            padding: 36px;
            border-radius: 12px;
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.08);
            max-width: 680px;
            width: 95%;
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        .button-group {
            display: flex;
            gap: 12px;
            justify-content: center;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        .control-button {
            text-decoration: none;  
            padding: 14px 22px;
            font-size: 1.1em;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.18s ease;
            color: white;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.08);
        }
        .on-button { background-color: #4CAF50; }
        .on-button:hover { transform: translateY(-2px); }
        .off-button { background-color: #f44336; }
        .off-button:hover { transform: translateY(-2px); }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 12px;
            margin-top: 12px;
        }
        .card {
            background-color: #f8f9fa;
            padding: 12px 14px;
            border-radius: 8px;
            text-align: left;
        }
        .label { font-weight: 700; color: #444; display:block; }
        .value { font-size: 1.25em; color: #222; margin-top:6px; }
        .voltage-ok { color: #4CAF50; font-weight: bold; }
        .voltage-danger { color: #f44336; font-weight: bold; }
        .current-ok { color: #4CAF50; font-weight:bold; }
        .current-warning { color: #ff9800; font-weight:bold; }
        .footer { color: #666; font-size: 0.85em; margin-top: 16px; }
        @media (max-width: 600px) {
            .status-grid { grid-template-columns: 1fr; }
            .control-button { width: 100%; }
        }
    </style>
    <script>
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('voltage-rms').textContent = data.voltage_rms.toFixed(1) + ' V';
                    document.getElementById('current-rms').textContent = data.current_rms.toFixed(2) + ' A';
                    document.getElementById('temperature').textContent = data.temperature.toFixed(1) + ' °C';
                    document.getElementById('power').textContent = data.power.toFixed(1) + ' W';
                    document.getElementById('energy').textContent = data.energy.toFixed(3) + ' kWh';
                    document.getElementById('cost').textContent = 'R$ ' + data.cost.toFixed(2);
                    document.getElementById('relay-state').textContent = data.relay_state ? 'LIGADO' : 'DESLIGADO';
                    document.getElementById('relay-state').style.color = data.relay_state ? '#4CAF50' : '#f44336';
                })
                .catch(error => {
                    console.error('Erro ao atualizar status:', error);
                });
        }
        setInterval(updateStatus, 2000);
        document.addEventListener('DOMContentLoaded', updateStatus);
    </script>
</head>
<body>
    <div class="container">
        <h1>💡 Smart Plug ESP32</h1>
        <div class="button-group">
            <a href="/on" class="control-button on-button">LIGAR</a>
            <a href="/off" class="control-button off-button">DESLIGAR</a>
        </div>
        <div class="status-grid">
            <div class="card">
                <span class="label">Tensão</span>
                <span class="value" id="voltage-rms">-- V</span>
            </div>
            <div class="card">
                <span class="label">Corrente</span>
                <span class="value" id="current-rms">-- A</span>
            </div>
            <div class="card">
                <span class="label">Temperatura</span>
                <span class="value" id="temperature">-- °C</span>
            </div>
            <div class="card">
                <span class="label">Potência</span>
                <span class="value" id="power">-- W</span>
            </div>
            <div class="card">
                <span class="label">Consumo</span>
                <span class="value" id="energy">-- kWh</span>
            </div>
            <div class="card">
                <span class="label">Custo</span>
                <span class="value" id="cost">R$ --</span>
            </div>
            <div class="card">
                <span class="label">Estado</span>
                <span class="value" id="relay-state">--</span>
            </div>
        </div>
        <p class="footer">ESP32 Smart Plug by Manus</p>
    </div>
</body>
</html>
)raw";

void handleRoot() {
    server.send(200, "text/html", HTML_CONTENT);
}

void handleOn() {
    RelayCommand_t command = COMMAND_ON;
    xQueueSend(xRelayCommandQueue, &command, 0);
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Turning on");
}

void handleOff() {
    RelayCommand_t command = COMMAND_OFF;
    xQueueSend(xRelayCommandQueue, &command, 0);
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Turning off");
}

void handleStatus() {
    VoltageData_t voltageData;
    CurrentData_t currentData;
    TemperatureData_t tempData;
    CostData_t costData;
    int currentRelayState;

    xQueuePeek(xVoltageDataQueue, &voltageData, 0);
    xQueuePeek(xCurrentDataQueue, &currentData, 0);
    xQueuePeek(xTemperatureDataQueue, &tempData, 0);
    xQueuePeek(xCostDataQueue, &costData, 0);

    xSemaphoreTake(xRelayStateMutex, portMAX_DELAY);
    currentRelayState = relayState;
    xSemaphoreGive(xRelayStateMutex);

    String json = "{";
    json += "\"voltage_rms\": " + String(voltageData.voltageRMS, 2) + ",";
    json += "\"current_rms\": " + String(currentData.currentRMS, 3) + ",";
    json += "\"temperature\": " + String(tempData.temperature, 1) + ",";
    json += "\"power\": " + String(costData.power, 1) + ",";
    json += "\"energy\": " + String(costData.energy, 3) + ",";
    json += "\"cost\": " + String(costData.cost, 2) + ",";
    json += "\"relay_state\": " + String(currentRelayState == HIGH ? "true" : "false");
    json += "}";

    server.send(200, "application/json", json);
}

void vTaskServer(void* pvParameters) {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/on", HTTP_GET, handleOn);
    server.on("/off", HTTP_GET, handleOff);
    server.on("/status", HTTP_GET, handleStatus);

    server.begin();
    Serial.println("HTTP server started.");

    while (true) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}