// monitoring.cpp
#include "monitoring.h"
#include "monitoringMessage.h"
#include <ArduinoJson.h>                // Required for JSON handling
#include "loramesh/loraMeshService.h"   // Required for LoraMesher::getInstance()
#include "message/messageManager.h"     // Required for MessageManager::getInstance()
#include "gps/gpsService.h"             // Required for GPSService instance and methods
#include "config.h"                     // For MONITORING_UPDATE_DELAY, LED pins, GPS_ENABLED etc.
#include "esp_log.h"                    // For ESP_LOGx macros

// #include "sensor.h" // Include if sensor functions are needed elsewhere

static const char* MONITORING_TAG = "MonitoringService";

// --- Initialization and Control ---
void Monitoring::init() {
    ESP_LOGV(MONITORING_TAG, "Initializing monitoring");

    createMonitoringTask();
    startMonitoring();
}

void Monitoring::startMonitoring() {
    ESP_LOGI(MONITORING_TAG, "Starting monitoring task");
    running = true;
    // Notify the task to start running if it was waiting
    if (monitoring_TaskHandle != NULL) {
        xTaskNotifyGive(monitoring_TaskHandle);
    }
    ESP_LOGV(MONITORING_TAG, "Monitoring task notified to start");
}

void Monitoring::stopMonitoring() {
    ESP_LOGI(MONITORING_TAG, "Stopping monitoring task");
    running = false;
}

// --- Commands Implementation ---
String Monitoring::monitoringIdle() {
    digitalWrite(LED, LED_OFF);
    ESP_LOGV(MONITORING_TAG, "Monitoring state set to Idle locally");
    return "Monitoring Idle";
}

String Monitoring::monitoringIdle(uint16_t dst) {
    if (dst == LoraMesher::getInstance().getLocalAddress()) {
        ESP_LOGV(MONITORING_TAG, "Executing monitoringIdle locally for dst=%X", dst);
        return monitoringIdle();
    }

    ESP_LOGV(MONITORING_TAG, "Sending monitoringIdle command to %X", dst);
    DataMessage* msg = getMonitoringMessage(MonitoringState::mIdle, dst);
    MessageManager::getInstance().sendMessage(messagePort::LoRaMeshPort, msg);
    return "Monitoring Idle command sent";
}

String Monitoring::monitoringActive() {
    digitalWrite(LED, LED_ON); // Assumes LED and LED_ON are defined
    ESP_LOGV(MONITORING_TAG, "Monitoring state set to Active locally");
    return "Monitoring Active";
}

String Monitoring::monitoringActive(uint16_t dst) {
    if (dst == LoraMesher::getInstance().getLocalAddress()) {
        ESP_LOGV(MONITORING_TAG, "Executing monitoringActive locally for dst=%X", dst);
        return monitoringActive();
    }

    ESP_LOGV(MONITORING_TAG, "Sending monitoringActive command to %X", dst);
    DataMessage* msg = getMonitoringMessage(MonitoringState::mActive, dst);
    MessageManager::getInstance().sendMessage(messagePort::LoRaMeshPort, msg);
    return "Monitoring Active command sent";
}

// --- Message Handling Implementation ---

DataMessage* Monitoring::getDataMessage(JsonObject data) {
    // Allocate memory for the message. Use nothrow to check allocation success.
    MonitoringMessage* monitoringMessage = new (std::nothrow) MonitoringMessage();
    if (!monitoringMessage) {
         ESP_LOGE(MONITORING_TAG, "Failed to allocate memory for incoming MonitoringMessage!");
         return nullptr;
    }
    monitoringMessage->deserialize(data); // Populate from JSON
    // Estimate size (might not be accurate for dynamic String members)
    monitoringMessage->messageSize = sizeof(MonitoringMessage) - sizeof(DataMessageGeneric);
    ESP_LOGV(MONITORING_TAG, "Created MonitoringMessage from JSON");
    return ((DataMessage*) monitoringMessage); // Return base class pointer
}

DataMessage* Monitoring::getMonitoringMessage(MonitoringState state, uint16_t dst) {
    MonitoringMessage* monitoringMessage = new (std::nothrow) MonitoringMessage();
     if (!monitoringMessage) {
         ESP_LOGE(MONITORING_TAG, "Failed to allocate memory for outgoing command MonitoringMessage!");
         return nullptr;
    }
    monitoringMessage->messageSize = sizeof(MonitoringMessage) - sizeof(DataMessageGeneric); // Approx size
    monitoringMessage->monitoringState = state;
    monitoringMessage->appPortSrc = appPort::MonitoringApp; // Defined in messageService.h?
    monitoringMessage->appPortDst = appPort::MonitoringApp; // Destination app is also Monitoring
    monitoringMessage->addrSrc = LoraMesher::getInstance().getLocalAddress();
    monitoringMessage->addrDst = dst;
    ESP_LOGV(MONITORING_TAG, "Created Monitoring command message, state: %d, dst: %X", state, dst);
    return (DataMessage*) monitoringMessage;
}

void Monitoring::processReceivedMessage(messagePort port, DataMessage* message) {
    // Cast the incoming message
    MonitoringMessage* monitoringMessage = (MonitoringMessage*) message;
    ESP_LOGV(MONITORING_TAG, "Processing received Monitoring message with state: %d", monitoringMessage->monitoringState);

    switch (monitoringMessage->monitoringState) {
        case MonitoringState::mIdle:
            ESP_LOGI(MONITORING_TAG, "Received monitoringIdle command");
            monitoringIdle(); // Execute local action
            break;

        case MonitoringState::mActive:
            ESP_LOGI(MONITORING_TAG, "Received monitoringActive command");
            monitoringActive(); // Execute local action
            break;

        default:
            // Log unexpected state values if necessary
            ESP_LOGW(MONITORING_TAG, "Received message with unknown monitoring state: %d", monitoringMessage->monitoringState);
            break;
    }

}

String Monitoring::getJSON(DataMessage* message) {
    ESP_LOGV(MONITORING_TAG, "Generating JSON for MonitoringMessage");
    MonitoringMessage* monitoringMessage = (MonitoringMessage*) message;

    DynamicJsonDocument doc(3072); // Adjust size as needed
    JsonObject jsonObj = doc.to<JsonObject>();

    monitoringMessage->serialize(jsonObj);

    String jsonOutput;
    serializeJson(doc, jsonOutput); // Serialize the document to a String

    // Optional: Pretty print for debugging
    // serializeJsonPretty(doc, Serial);
    // Serial.println();

    ESP_LOGV(MONITORING_TAG, "Generated JSON length: %d. Free heap: %d", jsonOutput.length(), ESP.getFreeHeap());
    if (doc.overflowed()) {
        ESP_LOGE(MONITORING_TAG, "JSON document overflowed during serialization! Increase DynamicJsonDocument size.");
    }
    return jsonOutput;
}

// --- Task Implementation ---

void Monitoring::createMonitoringTask() {
    // Create the task, passing 'this' instance pointer
    int res = xTaskCreate(
        monitoringLoop,         // Function to implement the task
        "Monitoring Task",      // Name of the task
        4096,                   // Stack size in words (adjust if needed, especially with JSON)
        (void*) this,           // Task input parameter (pointer to Monitoring instance)
        2,                      // Priority of the task (adjust based on system needs)
        &monitoring_TaskHandle  // Task handle
    );

    if (res != pdPASS) {
        ESP_LOGE(MONITORING_TAG, "Failed to create monitoring task! Error code: %d", res);
    } else {
        ESP_LOGI(MONITORING_TAG, "Monitoring task created successfully.");
    }
}

// Static task loop function
void Monitoring::monitoringLoop(void* pvParameters) {
    // Cast the parameter back to the Monitoring instance pointer
    Monitoring* monitoringInstance = (Monitoring*) pvParameters;
    static String uartInputBuffer = ""; // Static buffer persists across loop iterations
    const int MAX_UART_BUFFER_SIZE = 2048; // Safety limit for the buffer

    ESP_LOGI(MONITORING_TAG, "Monitoring task started execution.");

    while (true) { // Loop forever
        if (!monitoringInstance->running) {
            ESP_LOGI(MONITORING_TAG, "Monitoring task pausing.");
            // Wait indefinitely until notified by startMonitoring()
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            ESP_LOGI(MONITORING_TAG, "Monitoring task resumed.");
            // Optionally clear buffer on resume:
            // uartInputBuffer = "";
        } else {
            // --- Read Serial1 Data (for sensor JSON) ---
            while (Serial1.available()) {
                char incomingByte = Serial1.read();
                if (incomingByte == '\n' || incomingByte == '\r') {
                     uartInputBuffer.trim(); // Remove leading/trailing whitespace
                    if (uartInputBuffer.length() > 0) {
                        // Basic check: does it look like a JSON object?
                        if (uartInputBuffer.startsWith("{") && uartInputBuffer.endsWith("}")) {
                            monitoringInstance->currentSensorJsonData = uartInputBuffer;
                            ESP_LOGD(MONITORING_TAG, "Stored UART JSON data (len %d): %s", uartInputBuffer.length(), monitoringInstance->currentSensorJsonData.c_str());
                        } else {
                            ESP_LOGW(MONITORING_TAG, "Received non-JSON-object data from UART? Discarding: %s", uartInputBuffer.c_str());

                        }
                    } else {
                         ESP_LOGV(MONITORING_TAG, "Received empty line from UART.");
                    }
                    uartInputBuffer = ""; // Reset buffer for the next message
                } else if (incomingByte >= 32) { // Accumulate printable characters
                    if (uartInputBuffer.length() < MAX_UART_BUFFER_SIZE) {
                         uartInputBuffer += incomingByte;
                    } else {
                        // Prevent buffer overflow
                        ESP_LOGW(MONITORING_TAG, "UART input buffer overflow, discarding data. Resetting buffer.");
                        uartInputBuffer = ""; // Reset buffer on overflow
                    }
                }
                // Non-printable characters (other than newline) are ignored
            }
            // --- End Reading Serial1 ---

            uint32_t freeHeap = ESP.getFreeHeap();
            if (freeHeap > 20000) { // Set a reasonable threshold
                 ESP_LOGV(MONITORING_TAG, "Creating and sending monitoring status. Free heap: %d", freeHeap);
                 monitoringInstance->createAndSendMonitoring(); // Gather data and send
                 ESP_LOGV(MONITORING_TAG, "Monitoring status sent. Free heap after send: %d", ESP.getFreeHeap());
            } else {
                 ESP_LOGE(MONITORING_TAG, "Low heap memory (%d bytes), skipping monitoring send cycle.", freeHeap);
                 // Consider logging less frequently if heap remains low
            }

            // --- Delay ---
            // Use the delay value from config.h
            vTaskDelay(pdMS_TO_TICKS(MONITORING_UPDATE_DELAY)); // Use pdMS_TO_TICKS for clarity/portability
        }
    }
}

// --- Message Creation and Sending ---

void Monitoring::createAndSendMonitoring() {
    // Allocate message object using new (use nothrow for safer allocation check)
    MonitoringMessage* message = new (std::nothrow) MonitoringMessage();

    // Check if allocation succeeded
    if (!message) {
        ESP_LOGE(MONITORING_TAG, "Failed to allocate memory for monitoring message!");
        return; // Exit if allocation failed
    }

    ESP_LOGV(MONITORING_TAG, "Populating monitoring message for MQTT (including GPS)");

    // --- Get GPS Data ---
#ifdef GPS_ENABLED // Only attempt if GPS feature is compiled in
    GPSService& gpsService = GPSService::getInstance(); // Get GPS service instance

    // Fetch the GPS data structure first. getGPSMessage() handles waiting.
    message->gpsData = gpsService.getGPSMessage(); // Populate the gpsData member

    // NOW, check if the data *we just fetched* and stored in message->gpsData is valid.
    if (gpsService.isGPSValid(&message->gpsData)) {
        ESP_LOGD(MONITORING_TAG, "Fetched valid GPS data (Lat:%.6f) for monitoring message.", message->gpsData.latitude);
    } else {
        // GPS is enabled, but the data returned by getGPSMessage() wasn't valid.
        // This could be stale data or default zeros if no fix ever occurred.
        ESP_LOGW(MONITORING_TAG, "GPS data fetched for message is invalid (Lat:%.6f). Sending monitoring message with potentially stale/default GPS info.", message->gpsData.latitude);
    }
#else
    // GPS feature is not compiled in
    ESP_LOGV(MONITORING_TAG, "GPS Service disabled, not adding GPS data to monitoring message.");
    // message->gpsData will contain default values from its constructor
#endif // GPS_ENABLED


    // --- Populate Header Fields ---
    message->appPortDst = appPort::MQTTApp;           // Destination is the MQTT Application Port
    message->appPortSrc = appPort::MonitoringApp;     // Source is this Monitoring Application
    message->addrSrc = LoraMesher::getInstance().getLocalAddress(); // This node's LoRa address
    message->addrDst = 0;                             // Address 0 typically signifies uplink/broker
    message->messageId = monitoringId++;              // Increment the local monitoring message counter
    message->monitoringSendTimeInterval = MONITORING_UPDATE_DELAY; // Record the update interval

    // --- Populate Monitoring Status Fields using Helper Getters ---
    message->nServices = getServices();               // Get number of active services
    message->nRoutes = getRoutes();                   // Get number of LoRa routes
    message->routeTable = getRoutingTable();          // Get routing table as a String (potential size issue)
    message->ledStatus = getLEDstatus();              // Get current LED status
    message->outMessages = getOutMessages();          // Get approximate outgoing message count
    message->inMessages = getInMessages();            // Get approximate incoming message count


    message->sensorDataJson = currentSensorJsonData; // Assign the stored JSON string

    // --- Send via Message Manager ---
    ESP_LOGD(MONITORING_TAG, "Sending Monitoring message (ID: %d) to MessageManager for MqttPort.", message->messageId);

    MessageManager::getInstance().sendMessage(messagePort::MqttPort, (DataMessage*) message);

    ESP_LOGV(MONITORING_TAG, "Monitoring message (ID: %d) passed to MessageManager. Expecting MQTT service/port to handle deletion.", message->messageId);
}

void Monitoring::getJSONDataObject(JsonObject& doc, MonitoringMessage* monitoringMessage) {
    monitoringMessage->serialize(doc);
}

// --- Helper Getter Implementations ---
int Monitoring::getServices() {
    return MessageManager::getInstance().getActiveServices().size();
}

String Monitoring::getRoutingTable() {
    LM_LinkedList<RouteNode>* routingTableList = LoRaMeshService::getInstance().radio.routingTableListCopy();
    if (!routingTableList) {
        ESP_LOGE(MONITORING_TAG, "Failed to get routing table copy.");
        return "Error:NoTable"; // Return concise error string
    }

    String routeTableStr = "";
    routingTableList->setInUse(); // Mark the list as in use (required by LoRaMesher library)
    int len = routingTableList->getLength();
    ESP_LOGV(MONITORING_TAG, "Routing table length: %d", len);

    if (len > 0) {
        bool first = true;
        do { // Use do-while because getCurrent() might be valid before first next()
            RouteNode *rtn = routingTableList->getCurrent();
            if (!rtn) { // Safety check
                 ESP_LOGW(MONITORING_TAG, "Got NULL RouteNode in routing table iteration.");
                 continue; // Skip this entry
            }
            String entry = String(rtn->networkNode.address, HEX); // Use HEX for address clarity
            entry += "(" + String(rtn->networkNode.metric) + ")via";
            entry += String(rtn->via, HEX); // Use HEX for address clarity

            if (!first) {
                routeTableStr += ","; // Add separator
            }
            routeTableStr += entry;
            first = false;

        } while (routingTableList->next()); // Move to the next node
    } else {
        routeTableStr = "NoRoutes"; // Concise representation
    }

    routingTableList->releaseInUse(); // Release the list (required by LoRaMesher library)
    delete routingTableList; // <<< IMPORTANT: Delete the copy to prevent memory leak >>>

    ESP_LOGV(MONITORING_TAG, "Generated routing table string (len %d): %s", routeTableStr.length(), routeTableStr.c_str());

    return routeTableStr;
}

int Monitoring::getRoutes() {
    // Get a COPY just to find the length safely
    LM_LinkedList<RouteNode>* routingTableList = LoRaMeshService::getInstance().radio.routingTableListCopy();
    if (!routingTableList) {
         ESP_LOGE(MONITORING_TAG, "Failed to get routing table copy for count.");
         return 0; // Return 0 on error
    }
    int length = routingTableList->getLength();
    delete routingTableList; // <<< IMPORTANT: Delete the copy to prevent memory leak >>>
    ESP_LOGV(MONITORING_TAG, "Number of routes: %d", length);
    return length;
}

int Monitoring::getLEDstatus() {
    int state = led.getState(); // Using the cached reference 'led' from private members
    ESP_LOGV(MONITORING_TAG, "Current LED state: %d", state);
    return state;
}

int Monitoring::getOutMessages() {

    Query& query = Query::getInstance();
    int outCount = query.getQueryID() + (int)monitoringId;
    ESP_LOGV(MONITORING_TAG, "Outgoing message count (approx QueryID + MonID): %d", outCount);
    return outCount;
}

int Monitoring::getInMessages() {
    Query& query = Query::getInstance();
    int inCount = query.getInQuery();
    ESP_LOGV(MONITORING_TAG, "Incoming message count (approx QueryIn): %d", inCount);
    return inCount;
}