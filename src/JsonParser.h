#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <ArduinoJson.h>
#include <Arduino.h>

class JsonParser {
public:
    // Use JsonDocument instead of DynamicJsonDocument
    JsonParser(const char* json);
    JsonParser(const String& json);
    
    bool parse();  // Parse method
    
    // Sensor data getters
    float getIndoorSensor() const;
    float getOutdoorSensor() const;
    int getWaterHeaterSensor() const;
    int getWaterLevelMonitor() const;

private:
    String jsonData;  // Store JSON data
    bool parsed = false;
    JsonDocument doc;  // No size specified here
};

#endif
