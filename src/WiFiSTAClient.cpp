#include "WiFiSTAClient.h"

WiFiSTAClient::WiFiSTAClient(const char* ssid, const char* password, const char* server_ip, int server_port)
    : ssid(ssid), password(password), server_ip(server_ip), server_port(server_port), reconnectAttempts(10) {}

void WiFiSTAClient::begin() {
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < reconnectAttempts) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
    } else {
        Serial.println("Failed to connect to WiFi.");
    }
}

void WiFiSTAClient::disconnectFromAP() {
    WiFi.disconnect();
    Serial.println("Disconnected from WiFi.");
}

bool WiFiSTAClient::connectToServer() {
    if (!client.connect(server_ip, server_port)) {
        Serial.println("Connection to server failed.");
        return false;
    }
    Serial.println("Connected to server.");
    return true;
}

void WiFiSTAClient::disconnectFromServer() {
    if (client.connected()) {
        client.stop();
        Serial.println("Disconnected from server.");
    }
}

void WiFiSTAClient::sendData(const String& data) {
    if (client.connected()) {
        client.println(data);
    } else {
        Serial.println("Client not connected.");
    }
}

String WiFiSTAClient::receiveData() {
    String response = "";
    if (client.connected() && client.available()) {
        while (client.available()) {
            response += (char)client.read();
        }
    } else {
        Serial.println("No data available or client not connected.");
    }
    return response;
}

bool WiFiSTAClient::isAPConnected() {
    return WiFi.status();
}

bool WiFiSTAClient::isServerConnected() {
    return client.connected();
}

void WiFiSTAClient::setReconnectAttempts(int attempts) {
    reconnectAttempts = attempts;
}

void WiFiSTAClient::handleReconnect() {
    if (!isAPConnected()) {
        begin();
    }
    if (!isServerConnected()) {
        connectToServer();
    }
}
