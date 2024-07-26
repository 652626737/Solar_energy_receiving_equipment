#ifndef WIFISTACLIENT_H
#define WIFISTACLIENT_H

#include <WiFi.h>
#include <WiFiClient.h>

class WiFiSTAClient
{
public:
    WiFiSTAClient(const char *ssid, const char *password, const char *server_ip, int server_port);

    void begin();
    void disconnectFromAP();

    bool connectToServer();
    void disconnectFromServer();

    void sendData(const String &data);
    String receiveData();

    bool isAPConnected();
    bool isServerConnected();

    void setReconnectAttempts(int attempts);
    void handleReconnect();

private:
    const char *ssid;
    const char *password;
    const char *server_ip;
    int server_port;
    WiFiClient client;
    int reconnectAttempts;
};

#endif // WIFISTACLIENT_H
