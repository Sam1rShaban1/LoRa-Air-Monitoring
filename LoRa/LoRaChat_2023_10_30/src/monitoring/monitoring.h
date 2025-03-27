// monitoring.h
#pragma once

#include <Arduino.h>
#include <ArduinoLog.h> // Using ESP_LOGx is also fine, choose one standard
#include "message/messageService.h"
#include "message/messageManager.h"
#include "monitoring/monitoringMessage.h"
#include "monitoring/monitoringCommandService.h"
#include "config.h" // Contains definitions like MONITORING_UPDATE_DELAY, LED pins etc.
#include "led/led.h"
#include "query/query.h" // For message counting
// #include "wallet/wallet.h" // Include only if signature functionality is re-enabled

// Forward declaration for LoRa structures if needed by headers included later
// struct RouteNode;

class Monitoring: public MessageService{
public:
    /**
     * @brief Gets the singleton instance of the Monitoring service.
     * @return Reference to the Monitoring instance.
     */
    static Monitoring& getInstance(){
        static Monitoring instance;
        return instance;
    }

    /**
     * @brief Initializes the Monitoring service and creates its task.
     *        Ensure Serial1 is initialized before calling this.
     */
    void init();

    /**
     * @brief Starts the periodic monitoring task.
     */
    void startMonitoring();

    /**
     * @brief Stops the periodic monitoring task.
     */
    void stopMonitoring();

    // --- Monitoring State Commands ---

    /**
     * @brief Sets the local monitoring state to Idle (e.g., turns LED off).
     * @return Status string.
     */
    String monitoringIdle();

    /**
     * @brief Sends a command to set a remote node's monitoring state to Idle.
     * @param dst Destination node address.
     * @return Status string.
     */
    String monitoringIdle(uint16_t dst);

    /**
     * @brief Sets the local monitoring state to Active (e.g., turns LED on).
     * @return Status string.
     */
    String monitoringActive();

    /**
     * @brief Sends a command to set a remote node's monitoring state to Active.
     * @param dst Destination node address.
     * @return Status string.
     */
    String monitoringActive(uint16_t dst);


    // --- Message Handling ---

    /**
     * @brief Gets the total count of outgoing messages (approximate).
     * @return Number of outgoing messages.
     */
    int getOutMessages();

    /**
     * @brief Creates the monitoring status message and sends it via MessageManager.
     */
    void createAndSendMonitoring();

    /**
     * @brief Converts a MonitoringMessage into a JSON string format suitable for MQTT.
     * @param message Pointer to the DataMessage (should be a MonitoringMessage).
     * @return JSON string representation.
     */
    String getJSON(DataMessage* message);

    /**
     * @brief Creates a MonitoringMessage object from a JSON object.
     * @param data JsonObject containing the message data.
     * @return Pointer to the created DataMessage (cast from MonitoringMessage).
     */
    DataMessage* getDataMessage(JsonObject data);

    /**
     * @brief Creates a command MonitoringMessage to send state changes.
     * @param state The MonitoringState to send (mIdle or mActive).
     * @param dst The destination node address.
     * @return Pointer to the created DataMessage (cast from MonitoringMessage).
     */
    DataMessage* getMonitoringMessage(MonitoringState state, uint16_t dst);

    /**
     * @brief Processes received messages intended for the MonitoringApp port.
     * @param port The port the message arrived on.
     * @param message Pointer to the received DataMessage.
     */
    void processReceivedMessage(messagePort port, DataMessage* message);


    // --- Public Members ---
    MonitoringCommandService* monitoringCommandService = new MonitoringCommandService();


private:
    // --- Private Constructor (Singleton) ---
    Monitoring(): MessageService(MonitoringApp, "Monitoring") {
        commandService = monitoringCommandService;
        currentSensorJsonData = "{}"; // Default to empty JSON object or provide a sensible default
    };

    // --- Private Members ---
    TaskHandle_t monitoring_TaskHandle = NULL; // Handle for the monitoring task
    bool running = false;                      // Flag to control the task loop
    uint8_t monitoringId = 0;                  // Counter for outgoing monitoring messages
    Led& led = Led::getInstance();             // Reference to the LED singleton
    String currentSensorJsonData;              // Stores the latest JSON string from Serial1


    // --- Private Methods ---

    /**
     * @brief Creates the FreeRTOS task for monitoring.
     */
    void createMonitoringTask();

    /**
     * @brief The main loop function for the monitoring task (static for xTaskCreate).
     * @param pvParameters Pointer to the Monitoring instance.
     */
    static void monitoringLoop(void* pvParameters);

    // --- Signing (Optional - Implement if needed) ---
    // void signData(MonitoringMessage *message);
    void getJSONDataObject(JsonObject& doc, MonitoringMessage* monitoringdataMessage);
    // void getJSONSignObject(JsonObject& doc, MonitoringMessage* monitoringdataMessage);

    // --- Helper Getters for Status Information ---
    int getServices();
    String getRoutingTable();
    int getRoutes();
    int getLEDstatus();
    int getInMessages();
};