#include "WiFiAPServer.h"

/**
 * @brief WiFiAPServer 构造函数
 *
 * 初始化 WiFiAPServer 对象，设置 SSID、密码和端口号。
 *
 * @param ssid SSID 名称
 * @param password 密码
 * @param port 端口号
 */
WiFiAPServer::WiFiAPServer(const char *ssid, const char *password, int port)
    : ssid(ssid), password(password), port(port), server(port), clientConnected(false) {}

/**
 * @brief 启动 WiFi 接入点（AP）服务器
 *
 * 开始启动 WiFi 接入点（AP）服务器，并设置 SSID 和密码。
 * 启动后，将打印出 AP 的 IP 地址，并启动服务器。
 */
void WiFiAPServer::begin()
{
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin();
    Serial.println("Server started");
}

/**
 * @brief 处理客户端连接
 *
 * 用于处理与Wi-Fi接入点(AP)服务器连接的客户端请求。
 * 如果没有客户端连接，则监听新的客户端连接；
 * 如果已有客户端连接，则读取客户端发送的数据并打印到串行端口。
 */
void WiFiAPServer::handleClient()
{
    if (!clientConnected)
    {
        client = server.available(); // listen for incoming clients
        if (client)
        {
            Serial.println("New client connected");
            clientConnected = true;
        }
    }
    else
    {
        if (client.connected())
        {
            if (client.available())
            {
                lastReceivedData = client.readStringUntil('\n');
                Serial.print("Received: ");
                Serial.println(lastReceivedData);
            }
        }
        else
        {
            Serial.println("Client disconnected");
            client.stop();
            clientConnected = false;
        }
    }
}

/**
 * @brief 停止WiFi接入点服务器
 *
 * 停止当前运行的WiFi接入点服务器，并输出一条消息到串行端口，表示服务器已停止。
 */
void WiFiAPServer::stop()
{
    server.stop();
    Serial.println("Server stopped");
}

/**
 * @brief 判断是否有客户端连接
 *
 * 检查当前 WiFi 热点服务器是否有客户端连接。
 *
 * @return 如果有客户端连接则返回 true，否则返回 false
 */
/**
 * @brief 判断是否有客户端连接
 *
 * 检查 WiFiAPServer 是否已连接至少一个客户端。
 *
 * @return 如果至少有一个客户端连接，则返回 true；否则返回 false。
 */
bool WiFiAPServer::isClientConnected()
{
    return clientConnected;
}

/**
 * @brief 接收数据
 *
 * 从已连接的客户端接收数据，并返回接收到的字符串（以换行符 '\n' 为结束符）。
 *
 * @return 接收到的数据字符串，如果客户端未连接或没有数据可用，则返回空字符串。
 */
String WiFiAPServer::receiveData()
{
    if (client.connected() && client.available())
    {
        return client.readStringUntil('\n');

    }
    return "";
}

/**
 * @brief 发送数据
 *
 * 向已连接的客户端发送指定的数据。
 *
 * @param data 要发送的数据
 */
void WiFiAPServer::sendData(const String &data)
{
    if (client.connected())
    {
        client.print(data);
    }
}
String WiFiAPServer::getLastReceivedData() {
    return lastReceivedData;
}