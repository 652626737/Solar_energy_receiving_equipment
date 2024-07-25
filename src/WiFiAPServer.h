#ifndef WIFIAPSERVER_H
#define WIFIAPSERVER_H

#include <WiFi.h>

class WiFiAPServer {
public:
    WiFiAPServer(const char* ssid, const char* password, int port);
    
    void begin();
    void handleClient();
    void stop();

    bool isClientConnected();
    String receiveData();
    void sendData(const String& data);
    String getLastReceivedData(); // 新增的方法

private:
    const char* ssid;
    const char* password;
    int port;
    WiFiServer server;
    WiFiClient client;
    bool clientConnected;
    String lastReceivedData; // 将全局变量变为类的成员变量
};

#endif // WIFIAPSERVER_H
