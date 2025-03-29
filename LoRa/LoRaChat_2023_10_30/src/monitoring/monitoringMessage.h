#pragma once

#include <Arduino.h>
#include <ArduinoLog.h>    // Include if using ArduinoLog macros
#include <ArduinoJson.h>   // Required for JSON serialization/deserialization
#include "message/dataMessage.h" // Base class definition
#include "gps/gpsMessage.h" // Include GPSMessage struct definition

#pragma pack(push, 1)

enum MonitoringState : uint8_t {
    mIdle = 0,      // Monitoring is idle/waiting
    mActive = 1     // Monitoring is active
    // Add more states if needed for other commands/reports
};

class MonitoringMessage : public DataMessageGeneric {
public:
    // --- Fields ---

    // Common field for commands and potentially status reports
    MonitoringState monitoringState = MonitoringState::mIdle; // Default state

    // Fields primarily for periodic status reports sent to MQTT
    int monitoringSendTimeInterval = 0; // Interval (ms) at which this report was generated
    uint16_t nServices = 0;             // Number of active services
    uint16_t nRoutes = 0;               // Number of known routes
    uint16_t ledStatus = 0;             // Status of the LED (e.g., 0=OFF, 1=ON)
    uint16_t outMessages = 0;           // Approximate count of outgoing messages
    uint16_t inMessages = 0;            // Approximate count of incoming messages
    String routeTable;                  // String representation of the routing table
    GPSMessage gpsData;                 // Holds the GPS data structure
    String sensorDataJson = "{}";       // Stores sensor data, ideally as a JSON string

    void serialize(JsonObject& doc) {
        ((DataMessageGeneric*)(this))->serialize(doc);

        // Add Monitoring specific fields intended for the status report
        doc["monitoringState"] = monitoringState; // Include current state
        doc["intervalMs"] = monitoringSendTimeInterval;
        doc["nServices"] = nServices;
        doc["nRoutes"] = nRoutes;
        doc["routeTable"] = routeTable;         // Include routing table string
        doc["ledStatus"] = ledStatus;
        doc["outMessages"] = outMessages;
        doc["inMessages"] = inMessages;

        if (!sensorDataJson.isEmpty() && sensorDataJson != "{}") {

           DynamicJsonDocument sensorJsonDoc(1024); // Size might need adjustment based on expected sensor data size
           DeserializationError error = deserializeJson(sensorJsonDoc, sensorDataJson);

           if (error) {

               static const char* MM_TAG = "MonitoringMessage";
               ESP_LOGW(MM_TAG, "Failed to parse sensorDataJson: %s. Sending as raw string.", error.c_str());
               doc["sensorData"] = sensorDataJson;
           } else {
               doc["sensorData"] = sensorJsonDoc.as<JsonVariant>();
           }
        } else {
           doc["sensorData"] = JsonObject();
        }
        gpsData.serialize(doc);
    }


    void deserialize(JsonObject& doc) {
        ((DataMessageGeneric*)(this))->deserialize(doc);

        if (doc.containsKey("monitoringState") && doc["monitoringState"].is<uint8_t>()) {
            monitoringState = static_cast<MonitoringState>(doc["monitoringState"].as<uint8_t>());
        }

        if (doc.containsKey("sensorData")) {
            if (doc["sensorData"].is<JsonObject>() || doc["sensorData"].is<JsonArray>()) {
                 serializeJson(doc["sensorData"], sensorDataJson); // Store received JSON as string
            } else if (doc["sensorData"].is<const char*>()) {
                 sensorDataJson = doc["sensorData"].as<String>(); // Store received string
            } else {
                sensorDataJson = "{}";
            }
        }
    }
};

#pragma pack(pop)