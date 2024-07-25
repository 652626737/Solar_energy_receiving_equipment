#include "JsonParser.h"

JsonParser::JsonParser(const char* json) : jsonData(json) {}

JsonParser::JsonParser(const String& json) : jsonData(json) {}

bool JsonParser::parse() {
    DeserializationError error = deserializeJson(doc, jsonData);
    if (error) {
        parsed = false;  // Parsing failed
        return false;
    }
    parsed = true;  // Parsing successful
    return true;
}

float JsonParser::getIndoorSensor() const {
    if (!parsed) return NAN;
    return doc["indoorSensor"].as<float>();
}

float JsonParser::getOutdoorSensor() const {
    if (!parsed) return NAN;
    return doc["outdoorSensor"].as<float>();
}

int JsonParser::getWaterHeaterSensor() const {
    if (!parsed) return -1;
    return doc["waterheaterSensor"].as<int>();
}

int JsonParser::getWaterLevelMonitor() const {
    if (!parsed) return -1;
    return doc["waterLevelMonitor"].as<int>();
}
