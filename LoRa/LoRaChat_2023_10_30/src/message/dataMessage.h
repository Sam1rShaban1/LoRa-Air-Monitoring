#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>


#pragma pack(1)

// Message Ports
enum messagePort: uint8_t {
    LoRaMeshPort = 1,
    BluetoothPort = 2,
    WiFiPort = 3,
    MqttPort = 4
};

// App Ports
enum appPort: uint8_t {
    LoRaChat = 1,
    BluetoothApp = 2,
    WiFiApp = 3,
    GPSApp = 4,
    WalletApp = 5,
    CommandApp = 6,
    LoRaMesherApp = 7,
    MQTTApp = 8,
    SimApp = 12,
    LedApp = 13,
    SensorApp = 14,
    MetadataApp = 15,
    CdpApp = 30,
    FlkwsApp = 31,  // FF was 32
    EchoApp = 32,
    QueryApp = 41,
    MonitoringApp = 42
};

class DataMessageGeneric {
public:
    appPort appPortDst;
    appPort appPortSrc;
    uint8_t messageId;
    uint16_t addrSrc;
    uint16_t addrDst;
    String incomingData;
    uint32_t messageSize; // Message Size of the payload, not including header

    uint32_t getDataMessageSize() {
        return sizeof(DataMessageGeneric) + messageSize;
    }

    void serialize(JsonObject& doc) {
        doc["messageId"] = messageId;
        doc["addrSrc"] = addrSrc;
        doc["addrDst"] = addrDst;
        doc["messageSize"] = messageSize;
    }

    void deserialize(JsonObject& doc) {
        appPortDst = (appPort) doc["appPortDst"];
        appPortSrc = (appPort) doc["appPortSrc"];
        messageId = doc["messageId"];
        addrSrc = doc["addrSrc"];
        addrDst = doc["addrDst"];
        messageSize = doc["messageSize"];
    }
};

class DataMessage: public DataMessageGeneric {
public:
    uint8_t message[]; // Dynamic payload for message data
};

#pragma pack()
