#pragma once

#include <Arduino.h>

#include <ArduinoLog.h>

#include "message/dataMessage.h"

#pragma pack(1)

enum QueryCommand: uint8_t {
    sOff = 0,
    sOn = 1
};

class QueryMessage: public DataMessageGeneric {
public:
    QueryCommand queryCommand;
    u_int32_t queryvalue;

    void serialize(JsonObject& doc) {
        // Call the base class serialize function
        ((DataMessageGeneric*) (this))->serialize(doc);

        // Add the derived class data to the JSON object
        
        //doc["flkwsCommand"] = "Test2";
        //doc["query"] = "Test1";
        doc["queryCommand"] = queryCommand;
        doc["query"] = queryvalue;
        Log.verboseln(F("FF in class QueryMessage: void serialize flkwsCommand %d"),queryCommand );
        Log.verboseln(F("FF in class QueryMessage: void serialize queryvalue %d"),queryvalue );
        //JsonObject object = doc.to<JsonObject>();
        //JsonObject object = querydata.createNestedObject();
        //object["query"] = queryvalue;
        
    }

    void deserialize(JsonObject& doc) {
        // Call the base class deserialize function
        ((DataMessageGeneric*) (this))->deserialize(doc);

        // Add the derived class data to the JSON object
        Log.verboseln(F("FF in class QueryMessage: deserialize: seeing local variables in QueryMessage class"));

        queryCommand = doc["queryCommand"];
        Log.verboseln(F("FF in class QueryMessage: void deserialize %d"),queryCommand );
        queryvalue = doc["query"];
        Log.verboseln(F("FF in class QueryMessage: void deserialize %d"),queryvalue);
    }
};
#pragma pack()
