#include <WiFi.h>
#include <Arduino.h>
#include <PxMatrix.h>
#include "JsonParser.h"
#include "WiFiAPServer.h"
#include "esp_task_wdt.h"


// 看门狗超时时间，单位是秒
const int WDT_TIMEOUT = 5;

// WiFi接入点（AP）参数
const char *ap_ssid = "ESP32_AP";
const char *ap_password = "12345678";
const int ap_port = 80; // 服务器端口号

// Pins for LED MATRIX
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 2
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 60; // 30-70 is usually fine

// 创建一个WiFiAPServer对象
WiFiAPServer apServer(ap_ssid, ap_password, ap_port);
// 初始化 PxMatrix 实例
PxMATRIX display(64, 64, P_LAT, P_OE, P_A, P_B, P_C, P_D); // 根据实际连接情况修改参数

String lastReceivedData;
void IRAM_ATTR
display_updater()
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

    String NowReceivedData = apServer.getLastReceivedData();
    if(apServer.isClientConnected())
    {
        display.fillRect(0, 62, 2,2,display.color565(255, 0, 0));
    }
    else
    {
        display.fillRect(0, 62, 2,2,display.color565(0, 0, 0));
    }
    if (NowReceivedData != lastReceivedData)
    {

        JsonParser parser(NowReceivedData);
        if (parser.parse())
        {
            // Serial.print("Indoor Sensor: ");
            // Serial.println(parser.getIndoorSensor());
            // Serial.print("Outdoor Sensor: ");
            // Serial.println(parser.getOutdoorSensor());
            // Serial.print("Water Heater Sensor: ");
            // Serial.println(parser.getWaterHeaterSensor());
            // Serial.print("Water Level Monitor: ");
            // Serial.println(parser.getWaterLevelMonitor());
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

    // 初始化WiFi接入点（AP）服务器
    apServer.begin();

    // 打印AP的IP地址
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    // 初始化看门狗定时器
    esp_task_wdt_init(WDT_TIMEOUT, true);  // 设置看门狗超时时间
    esp_task_wdt_add(NULL);  // 添加当前任务到看门狗
}

void loop()
{
    // 处理客户端连接和通信
    apServer.handleClient();
    display_Main();
    
    // 重置看门狗定时器，防止看门狗超时重启
    esp_task_wdt_reset();
    // // 示例：每隔1秒向客户端发送一条消息
    // static unsigned long lastTime = 0;
    // if (millis() - lastTime > 1000) {
    //     lastTime = millis();
    //     if (apServer.isClientConnected()) {
    //         apServer.sendData("Hello from ESP32!\n");
    //     }
    // }
}
