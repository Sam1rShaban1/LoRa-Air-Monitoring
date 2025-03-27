// monitoringMessage.h
#pragma once

#include <Arduino.h>
#include <ArduinoLog.h>    // Include if using ArduinoLog macros
#include <ArduinoJson.h>   // Required for JSON serialization/deserialization
#include "message/dataMessage.h" // Base class definition
// #include "gps/gpsMessage.h" // Include if GPSMessage struct is used
// #include "sensorlto/signatureMessage.h" // Include if Signature related structs are used

// Use packed struct if sending raw bytes is *ever* a possibility,
// but be wary of complex types like String inside.
// If only using JSON, packing is less critical.
#pragma pack(push, 1)

/**
 * @brief Defines the possible states for the monitoring service/commands.
 */
enum MonitoringState : uint8_t {
    mIdle = 0,      // Monitoring is idle/waiting
    mActive = 1     // Monitoring is active
    // Add more states if needed for other commands/reports
};

/**
 * @brief Data structure for monitoring messages (both commands and status reports).
 * Inherits generic message fields from DataMessageGeneric.
 */
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
    // GPSMessage gps;                  // GPS data structure (if needed)

    // Holds the raw JSON string received from UART (or other sensor source)
    String sensorDataJson = "{}";       // Default to empty JSON object


    // --- Serialization (to JSON - used by getJSONDataObject) ---
    /**
     * @brief Serializes the relevant fields of this message into a JSON object.
     *        This is typically used when preparing the message for MQTT transmission.
     * @param doc The JsonObject to populate.
     */
    void serialize(JsonObject& doc) {
        // First, serialize the base class fields (addrSrc, addrDst, etc.)
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
        doc["sensorData"] = sensorDataJson;    // <<< Include the raw sensor JSON string

        // Example: Serialize GPS data if the struct exists and has a serialize method
        // JsonObject gpsData = doc.createNestedObject("gps");
        // gps.serialize(gpsData);
    }

    // --- Deserialization (from JSON - used by getDataMessage) ---
    /**
     * @brief Deserializes data from a JSON object into this message's fields.
     *        This is typically used when receiving a command or data via JSON.
     * @param doc The JsonObject containing the data.
     */
    void deserialize(JsonObject& doc) {
        // First, deserialize the base class fields
        ((DataMessageGeneric*)(this))->deserialize(doc);

        // Deserialize fields that might be set via incoming JSON (e.g., commands)
        if (doc.containsKey("monitoringState") && doc["monitoringState"].is<uint8_t>()) {
            monitoringState = static_cast<MonitoringState>(doc["monitoringState"].as<uint8_t>());
        }

        // Only deserialize other fields if they can be set externally via JSON
        // (Usually status fields like nRoutes, ledStatus etc. are generated locally)
        // Example: Allow external setting of interval for configuration?
        // if (doc.containsKey("intervalMs") && doc["intervalMs"].is<int>()) {
        //     monitoringSendTimeInterval = doc["intervalMs"].as<int>();
        // }

        // Example: Allow external setting of sensorDataJson (e.g., from MQTT)?
        // if (doc.containsKey("sensorData") && !doc["sensorData"].isNull()) {
        //     sensorDataJson = doc["sensorData"].as<String>(); // Use as<String>() or as<const char*>()
        // }
    }

    // Note: Removed update(), periodicUpdateAndSend(), sendMonitoringData(), lastSendTime
    // as this logic is now handled within the Monitoring class task loop.
};

#pragma pack(pop) // End structure packing