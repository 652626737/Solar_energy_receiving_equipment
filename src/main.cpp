#include <WiFi.h>
#include <Arduino.h>
#include <PxMatrix.h>
#include <HTTPClient.h>
#include "JsonParser.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "WiFiSTAClient.h"

const char *sta_ssid = "ESP32_AP";
const char *sta_password = "12345678";
const char *sta_server_ip = "192.168.4.1";
const int sta_server_port = 80;

// 定义WiFi网络参数
const char *ssid = "ESP32_AP";
const char *password = "12345678";
const char *serverUrl = "http://192.168.4.1/"; // 服务器的IP地址和端口

WiFiSTAClient Client(ssid, password, sta_server_ip, sta_server_port);
HTTPClient http;

// Pins for LED MATRIX
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 60; // 30-70 is usually fine

unsigned long clientLastHeartbeat = 0;
const unsigned long heartbeatTimeout = 30000;

// 初始化 PxMatrix 实例
PxMATRIX display(64, 64, P_LAT, P_OE, P_A, P_B, P_C, P_D); // 根据实际连接情况修改参数

String lastReceivedData;

void IRAM_ATTR display_updater()
{
    // Increment the counter and set the time of ISR
    portENTER_CRITICAL_ISR(&timerMux);
    display.display(display_draw_time);
    portEXIT_CRITICAL_ISR(&timerMux);
}

void display_update_enable(bool is_enable)
{

    if (is_enable)
    {
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, &display_updater, true);
        timerAlarmWrite(timer, 4000, true);
        timerAlarmEnable(timer);
    }
    else
    {
        timerDetachInterrupt(timer);
        timerAlarmDisable(timer);
    }
}

void display_Main()
{

    String NowReceivedData = lastReceivedData;
    if (Client.isAPConnected())
    {
        display.fillRect(0, 62, 2, 2, display.color565(255, 0, 0));
    }
    else
    {
        display.fillRect(0, 62, 2, 2, display.color565(0, 0, 0));
    }

    JsonParser parser(NowReceivedData);
    if (parser.parse())
    {
        Serial.print("Indoor Sensor: ");
        Serial.println(parser.getIndoorSensor());
        Serial.print("Outdoor Sensor: ");
        Serial.println(parser.getOutdoorSensor());
        Serial.print("Water Heater Sensor: ");
        Serial.println(parser.getWaterHeaterSensor());
        Serial.print("Water Level Monitor: ");
        Serial.println(parser.getWaterLevelMonitor());
        display.clearDisplay();
        display.setTextSize(0);
        display.setRotation(0);
        display.setTextColor(display.color565(255, 0, 0));
        // display.setCursor(0, 20);
        // display.println(parser.getIndoorSensor());
        display.setCursor(15, 56);
        display.println(parser.getOutdoorSensor());
        display.setTextSize(2);
        display.setCursor(20, 25);
        display.println(parser.getWaterHeaterSensor());

        if (parser.getWaterLevelMonitor() >= 100)

        {
            display.drawRect(0, 0, 64, 5, display.color565(255, 255, 255));
            display.fillRect(0, 0, 64, 5, display.color565(0, 255, 0));
        }
        else if (
            parser.getWaterLevelMonitor() >= 80 &&
            parser.getWaterLevelMonitor() < 100)
        {
            display.drawRect(0, 0, 64, 5, display.color565(255, 255, 255));
            display.fillRect(0, 0, 48, 5, display.color565(0, 255, 0));
        }
        else if (
            parser.getWaterLevelMonitor() >= 50 &&
            parser.getWaterLevelMonitor() < 80)
        {
            display.drawRect(0, 0, 64, 5, display.color565(255, 255, 255));
            display.fillRect(0, 0, 32, 5, display.color565(0, 255, 0));
        }
        else if (
            parser.getWaterLevelMonitor() >= 20 &&
            parser.getWaterLevelMonitor() < 50)
        {
            display.drawRect(0, 0, 64, 5, display.color565(255, 255, 255));
            display.fillRect(0, 0, 10, 5, display.color565(255, 0, 0));
        }
        else if (
            parser.getWaterLevelMonitor() >= 0 &&
            parser.getWaterLevelMonitor() < 20)
        {
            display.drawRect(0, 0, 64, 5, display.color565(255, 0, 0));
            display.fillRect(0, 0, 10, 5, display.color565(255, 0, 0));
        }
        lastReceivedData = NowReceivedData;
    }
    else
    {
        Serial.println("Failed to parse JSON.");
    }
}

void UpdateLastReceivedData()
{
    if (Client.isAPConnected() && Client.connectToServer())
    {
        http.begin(serverUrl);
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            // 如果HTTP请求成功，获取响应数据
            lastReceivedData = http.getString();
            Serial.println("Response:");
            Serial.println(lastReceivedData);

            // 解析JSON数据
            StaticJsonDocument<200> jsonDoc;
            DeserializationError error = deserializeJson(jsonDoc, lastReceivedData);

            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
            }

            // 提取数据
            int indoorSensor = jsonDoc["indoorSensor"];
            int outdoorSensor = jsonDoc["outdoorSensor"];
            int waterheaterSensor = jsonDoc["waterheaterSensor"];
            int waterLevelMonitor = jsonDoc["waterLevelMonitor"];

            // 打印数据
            Serial.println("Parsed Data:");
            Serial.print("Indoor Sensor: ");
            Serial.println(indoorSensor);
            Serial.print("Outdoor Sensor: ");
            Serial.println(outdoorSensor);
            Serial.print("Water Heater Sensor: ");
            Serial.println(waterheaterSensor);
            Serial.print("Water Level Monitor: ");
            Serial.println(waterLevelMonitor);
        }
        else
        {
            // 请求失败
            Serial.print("HTTP GET failed. Error code: ");
            Serial.println(httpCode);
        }

        // 结束HTTP请求
        http.end();
    }
    else
    {
        Client.begin();
    }
}

void Task_Display(void *pvParameters)
{
    while (true)
    {
        // 调用 display_Main 和 UpdateLastReceivedData
        UpdateLastReceivedData();
        display_Main();

        // 每隔 1000 毫秒执行一次
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
void setup()
{
    Serial.begin(115200);
    display.begin(16);
    display.setPanelsHeight(2);
    display.setBrightness(100);
    display.clearDisplay();
    display.setTextColor(display.color565(255, 0, 0));
    display.setCursor(2, 0);
    display.print("Pixel");
    display.setTextColor(display.color565(255, 0, 0));
    display.setCursor(2, 8);
    display.print("Time");
    display_update_enable(true);
    delay(3000);

    Client.begin();

    xTaskCreate(Task_Display, "DisplayTask", 2048, NULL, 1, NULL);
}

void loop()
{
}