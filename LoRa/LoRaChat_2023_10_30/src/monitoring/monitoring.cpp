// monitoring.cpp
#include "monitoring.h"
#include "monitoringMessage.h"
#include <ArduinoJson.h>     // Required for JSON handling
#include "loramesh/loraMeshService.h"       // Required for LoraMesher::getInstance()
#include "message/messageManager.h"  // Required for MessageManager::getInstance()
// #include "sensor.h"       // Include if sensor functions are needed elsewhere
// Include other necessary headers from your project structure

// Make sure ESP_LOGx levels are defined appropriately in your config/sdkconfig
#include "esp_log.h"

static const char* MONITORING_TAG = "MonitoringService"; // TAG for ESP_LOGx

// --- Initialization and Control ---
void Monitoring::init(){
    ESP_LOGV(MONITORING_TAG, "Initializing monitoring");
    // !!! IMPORTANT: Ensure Serial1 is initialized somewhere in your setup() !!!
    // Example: Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    createMonitoringTask();
    startMonitoring();
}

void Monitoring::startMonitoring(){
    ESP_LOGV(MONITORING_TAG, "Starting monitoring");
    running = true;
    // Notify the task to start running if it was waiting
    if (monitoring_TaskHandle != NULL) {
        xTaskNotifyGive(monitoring_TaskHandle);
    }
    ESP_LOGV(MONITORING_TAG, "Monitoring task notified to start");
}

void Monitoring::stopMonitoring() {
    ESP_LOGV(MONITORING_TAG, "Stopping monitoring");
    running = false;
    // Task will stop processing in its loop after the current cycle
}

// --- Commands Implementation ---
String Monitoring::monitoringIdle(){
    digitalWrite(LED, LED_OFF); // Assumes LED and LED_OFF are defined in config.h or led.h
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
    // Assuming LoRaMeshPort is defined in message/messageService.h or similar
    MessageManager::getInstance().sendMessage(messagePort::LoRaMeshPort, msg);
    // MessageManager should handle deletion of 'msg' after sending
    // delete msg; // Only uncomment if MM does *not* manage memory

    return "Monitoring Idle command sent";
}

String Monitoring::monitoringActive(){
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
    // MessageManager should handle deletion of 'msg' after sending
    // delete msg; // Only uncomment if MM does *not* manage memory

    return "Monitoring Active command sent";
}

// --- Message Handling Implementation ---

DataMessage* Monitoring::getDataMessage(JsonObject data) {
    MonitoringMessage* monitoringMessage = new MonitoringMessage(); // Use new
    monitoringMessage->deserialize(data);
    // Calculate size approximately. Might not be perfectly accurate with Strings.
    monitoringMessage->messageSize = sizeof(MonitoringMessage) - sizeof(DataMessageGeneric);
    ESP_LOGV(MONITORING_TAG, "Created MonitoringMessage from JSON");
    return ((DataMessage*) monitoringMessage);
}

DataMessage* Monitoring::getMonitoringMessage(MonitoringState state, uint16_t dst){
    MonitoringMessage* monitoringMessage = new MonitoringMessage(); // Use new
    monitoringMessage->messageSize = sizeof(MonitoringMessage) - sizeof(DataMessageGeneric); // Approx size
    monitoringMessage->monitoringState = state;
    monitoringMessage->appPortSrc = appPort::MonitoringApp; // Defined in messageService.h?
    monitoringMessage->appPortDst = appPort::MonitoringApp; // Destination app is also Monitoring
    monitoringMessage->addrSrc = LoraMesher::getInstance().getLocalAddress();
    monitoringMessage->addrDst = dst;
    ESP_LOGV(MONITORING_TAG, "Created Monitoring command message, state: %d, dst: %X", state, dst);
    return (DataMessage*) monitoringMessage;
}

void Monitoring::processReceivedMessage(messagePort port, DataMessage* message){
    // Cast the incoming message
    MonitoringMessage* monitoringMessage = (MonitoringMessage*) message;
    ESP_LOGV(MONITORING_TAG, "Processing received message with state: %d", monitoringMessage->monitoringState);

    switch(monitoringMessage->monitoringState){
        case MonitoringState::mIdle:
            ESP_LOGV(MONITORING_TAG, "Received monitoringIdle command");
            monitoringIdle(); // Execute local action
            break; // <<< FIXED: Added break;

        case MonitoringState::mActive:
            ESP_LOGV(MONITORING_TAG, "Received monitoringActive command");
            monitoringActive(); // Execute local action
            break; // <<< FIXED: Added break;

        default:
            // Log unexpected state values if necessary
            ESP_LOGW(MONITORING_TAG, "Received message with unknown monitoring state: %d", monitoringMessage->monitoringState);
            break;
    }
    // MessageManager should handle deleting the original received 'message' after processing
}


String Monitoring::getJSON(DataMessage* message){
    ESP_LOGV(MONITORING_TAG, "Generating JSON for MonitoringMessage");
    MonitoringMessage* monitoringMessage = (MonitoringMessage*) message;
    // Adjust JSON document size based on max expected size (especially sensorDataJson and routeTable)
    DynamicJsonDocument doc(2048);
    JsonObject jsonObj = doc.to<JsonObject>();

    // Create the nested "data" object as per the original structure
    JsonObject dataObj = jsonObj.createNestedObject("data");
    getJSONDataObject(dataObj, monitoringMessage); // Populate the data object

    // Add signature if needed (currently commented out in header)
    // getJSONSignObject(jsonObj, monitoringMessage);

    String json;
    serializeJson(doc, json); // Serialize the document to a String
    // serializeJsonPretty(doc, Serial); // Optional: for debugging output to Serial
    ESP_LOGV(MONITORING_TAG, "Generated JSON length: %d", json.length());
    return json;
}

// --- Task and Loop Implementation ---

void Monitoring::createMonitoringTask(){
    // Create the task, passing 'this' instance pointer
    int res = xTaskCreate(
        monitoringLoop,         // Function to implement the task
        "Monitoring Task",      // Name of the task
        4096,                   // Stack size in words
        (void*) this,           // Task input parameter (pointer to Monitoring instance)
        2,                      // Priority of the task
        &monitoring_TaskHandle  // Task handle
    );

    if (res != pdPASS) {
        ESP_LOGE(MONITORING_TAG, "Failed to create monitoring task! Error code: %d", res);
    } else {
        ESP_LOGI(MONITORING_TAG, "Monitoring task created successfully.");
    }
}

void Monitoring::monitoringLoop(void* pvParameters) {
    // Cast the parameter back to the Monitoring instance pointer
    Monitoring* monitoringInstance = (Monitoring*) pvParameters;
    static String uartInputBuffer = ""; // Static buffer to accumulate chars across loop iterations
    const int MAX_UART_BUFFER_SIZE = 1024; // Safety limit for the buffer

    ESP_LOGI(MONITORING_TAG, "Monitoring task started execution.");

    while (true) { // Loop forever
        if (!monitoringInstance->running) {
            ESP_LOGI(MONITORING_TAG, "Monitoring task pausing.");
            // Wait indefinitely until notified by startMonitoring()
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            ESP_LOGI(MONITORING_TAG, "Monitoring task resumed.");
        } else {
            // --- Read Serial1 Data ---
            while (Serial1.available()) {
                char incomingByte = Serial1.read();
                if (incomingByte == '\n') { // Check for newline - end of message?
                    uartInputBuffer.trim(); // Remove leading/trailing whitespace
                    if (uartInputBuffer.length() > 0) {
                        // Store the received string directly
                        monitoringInstance->currentSensorJsonData = uartInputBuffer;
                        ESP_LOGD(MONITORING_TAG, "Stored UART data (len %d): %s", uartInputBuffer.length(), monitoringInstance->currentSensorJsonData.c_str());
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
                // Non-printable characters other than '\n' are ignored
            }
            // --- End Reading Serial1 ---

            // --- Send Periodic Monitoring Status ---
            ESP_LOGV(MONITORING_TAG, "Creating and sending monitoring status. Free heap: %d", ESP.getFreeHeap());
            monitoringInstance->createAndSendMonitoring(); // Gather data and send
            ESP_LOGV(MONITORING_TAG, "Monitoring status sent. Free heap: %d", ESP.getFreeHeap());

            // --- Delay ---
            // Use the delay value from config
            vTaskDelay(MONITORING_UPDATE_DELAY / portTICK_PERIOD_MS);
        }
    }
}


void Monitoring::createAndSendMonitoring(){
    // Allocate message object using new
    MonitoringMessage* message = new MonitoringMessage();

    if (message) {
        ESP_LOGV(MONITORING_TAG, "Populating monitoring message for MQTT");

        // --- Populate Header Fields ---
        message->appPortDst = appPort::MQTTApp; // Destination is MQTT handler
        message->appPortSrc = appPort::MonitoringApp;
        message->addrSrc = LoraMesher::getInstance().getLocalAddress();
        message->addrDst = 0; // Typically 0 for messages destined for MQTT/gateway
        message->messageId = monitoringId++;
        // Size is approximate, especially with Strings involved
        message->messageSize = sizeof(MonitoringMessage) - sizeof(DataMessageGeneric);
        message->monitoringSendTimeInterval = MONITORING_UPDATE_DELAY;

        // --- Populate Status Fields using Helper Getters ---
        message->nServices = getServices();
        message->nRoutes = getRoutes();
        message->routeTable = getRoutingTable();
        message->ledStatus = getLEDstatus();
        message->outMessages = getOutMessages();
        message->inMessages = getInMessages();

        // --- <<< Copy the latest sensor data string >>> ---
        message->sensorDataJson = currentSensorJsonData; // Directly copy the stored string
        ESP_LOGD(MONITORING_TAG, "Adding sensor data to message: %s", message->sensorDataJson.c_str());

        // --- Send via Message Manager ---
        // Assuming MqttPort is defined and handled by another service
        MessageManager::getInstance().sendMessage(messagePort::MqttPort, (DataMessage*) message);

        // --- Memory Management ---
        // IMPORTANT: Assume MessageManager takes ownership or the MQTT service deletes the message.
        // If you experience memory leaks, you MUST delete the message *after* it has been fully processed/sent.
        // This might require changes in MessageManager or the receiving service.
        // delete message; // DO NOT uncomment unless you are SURE it's safe to do so here.

    } else {
        ESP_LOGE(MONITORING_TAG, "Failed to allocate memory for monitoring message!");
    }
}

// --- JSON Helper Implementation ---
void Monitoring::getJSONDataObject(JsonObject& doc, MonitoringMessage* monitoringMessage) {
    // Delegate the serialization of data fields to the message object itself
    monitoringMessage->serialize(doc);
}

// --- Signing (Implement if needed) ---
/*
void Monitoring::signData(MonitoringMessage* monitoringMessage) {
    // ... Create JSON object of data to sign ...
    // ... Call Wallet::getInstance().signJson(...) ...
    // ... Copy signature into message ...
}

void Monitoring::getJSONSignObject(JsonObject& doc, MonitoringMessage* monitoringMessage) {
    // ... Add signature fields to the main JSON document ...
}
*/

// --- Helper Getter Implementations ---
int Monitoring::getServices(){
    // Assuming getActiveServices() is thread-safe or returns a copy
    return MessageManager::getInstance().getActiveServices().size();
}

String Monitoring::getRoutingTable(){
    // Get a COPY of the routing table
    LM_LinkedList<RouteNode>* routingTableList = LoRaMeshService::getInstance().radio.routingTableListCopy();
    if (!routingTableList) {
        ESP_LOGE(MONITORING_TAG, "Failed to get routing table copy.");
        return "Error: No Table"; // Return error string
    }

    String routeTableStr = "";
    bool first = true;

    routingTableList->setInUse(); // Mark the list as in use (required by LoRaMesher library)
    int len = routingTableList->getLength();
    ESP_LOGV(MONITORING_TAG, "Routing table length: %d", len);

    if(len > 0) {
        do { // Use do-while because getCurrent() might be valid before next()
            RouteNode *rtn = routingTableList->getCurrent();
            if (!rtn) { // Safety check
                 ESP_LOGW(MONITORING_TAG, "Got NULL RouteNode in routing table iteration.");
                 continue;
            }
            // Build the string for this entry
            String entry = String(rtn->networkNode.address) + " (" + String(rtn->networkNode.metric) + ") via " + String(rtn->via);

            if (!first) {
                routeTableStr += ","; // Add separator
            }
            routeTableStr += entry;
            first = false;

        } while (routingTableList->next()); // Move to the next node
    } else {
        routeTableStr = "No routes";
    }

    routingTableList->releaseInUse(); // Release the list (required by LoRaMesher library)
    delete routingTableList; // <<< FIXED: Delete the copy to prevent memory leak

    ESP_LOGV(MONITORING_TAG, "Generated routing table string: %s", routeTableStr.c_str());
    return routeTableStr;
}

int Monitoring::getRoutes(){
    // Get a COPY to find the length
    LM_LinkedList<RouteNode>* routingTableList = LoRaMeshService::getInstance().radio.routingTableListCopy();
    if (!routingTableList) {
         ESP_LOGE(MONITORING_TAG, "Failed to get routing table copy for count.");
         return 0; // Return 0 on error
    }
    int length = routingTableList->getLength();
    delete routingTableList; // <<< FIXED: Delete the copy to prevent memory leak
    ESP_LOGV(MONITORING_TAG, "Number of routes: %d", length);
    return length;
}

int Monitoring::getLEDstatus(){
    // Assuming led.getState() returns the current state (e.g., 0 for OFF, 1 for ON)
    int state = led.getState();
    ESP_LOGV(MONITORING_TAG, "Current LED state: %d", state);
    return state;
}

int Monitoring::getOutMessages(){
    // This logic combines two counters - ensure this is the intended behavior
    Query& query = Query::getInstance();
    int outCount = query.getQueryID() + monitoringId;
    ESP_LOGV(MONITORING_TAG, "Outgoing message count (approx): %d", outCount);
    return outCount;
}

int Monitoring::getInMessages(){
    Query& query = Query::getInstance();
    int inCount = query.getInQuery();
    ESP_LOGV(MONITORING_TAG, "Incoming message count (approx): %d", inCount);
    return inCount;
}