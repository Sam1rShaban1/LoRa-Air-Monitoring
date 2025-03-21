#pragma once

#include <Arduino.h>
#include <ArduinoLog.h>
#include "message/dataMessage.h"
#include "gps/gpsMessage.h"
#include "sensorlto/signatureMessage.h"

#pragma pack(1)

// static const char* MONITORING_TAG = "MonitoringService";

enum MonitoringState: uint8_t {
    mIdle = 0,
    mActive = 1
};

class MonitoringMessage: public DataMessageGeneric {
public:
    MonitoringState monitoringState;
    int monitoringSendTimeInterval;
    uint16_t nAddress;
    uint16_t nServices;
    uint16_t nRoutes;
    String routeTable;
    uint16_t ledStatus;
    uint16_t outMessages;
    uint16_t inMessages;
    GPSMessage gps;
        
    uint32_t helloPacket[5] = {0, 1, 2, 3, 4};

    void serialize(JsonObject& doc) {
        //ESP_LOGV(MONITORING_TAG, "in class MonitoringMessage: void serialize");
        int i;
        ((DataMessageGeneric*)(this))->serialize(doc);
        doc["monitoringState"] = 1; // FF: for short monitoring message
        doc["nServices"] = nServices;
        doc["nRoutes"] = nRoutes;
        doc["ledStatus"] = ledStatus;
        doc["outMessages"] = outMessages;
        doc["inMessages"] = inMessages;
        // doc["routeTable"] = routeTable;
        // doc["latitude"] = gps.latitude;
        // doc["longitude"] = gps.longitude;
        // doc["hour"] = gps.hour;
        // doc["minute"] = gps.minute;
        // doc["second"] = gps.second;
        doc["incomingData"] = DataMessageGeneric::incomingData;
    }

    void deserialize(JsonObject& doc) {
        ((DataMessageGeneric*)(this))->deserialize(doc);
        monitoringState = doc["monitoringState"];
        Log.verbose(F("Monitoring in monitoringMessage.h deserialized %d"), monitoringState);

        if (doc.containsKey("incomingData")) {  // Corrected to match "incomingData"
            DataMessageGeneric::incomingData = doc["incomingData"].as<String>();  // Deserializing incomingData properly
        }
    }

    void update() {
        // Read from Serial1 in the update method
        if (Serial1.available()) {
            char incomingByte = Serial1.read();
            DataMessageGeneric::incomingData += incomingByte; // Concatenate incomingByte to incomingData (using inherited incomingData)
        }
    }
};

#pragma pack()
