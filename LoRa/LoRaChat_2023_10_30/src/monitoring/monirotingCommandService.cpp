// monitoringCommandService.cpp
#include "monitoringCommandService.h"
#include "monitoring.h" // <<< Ensure this is included and uncommented for MonitoringState and Monitoring::getInstance()

/**
 * @brief Constructor for MonitoringCommandService.
 *        Adds commands that interface with the Monitoring singleton instance.
 */
MonitoringCommandService::MonitoringCommandService() {

    // Command: /monitoringIdle <destination_address_hex>
    // Example usage: /monitoringIdle A1B2
    addCommand(Command(
        "/monitoringIdle", // Argument 1: Command name (const char*)
        "Set monitoring to Idle. Arg: destination node hex addr (e.g., A1B2)", // Argument 2: Description (const char*)
        MonitoringState::mIdle, // <<< FIX: Argument 3: State enum value (MonitoringState)
        1,                      // Argument 4: Number of arguments expected (int)
        [](String args) {       // Argument 5: Lambda function to execute
            // Parse the hex address string from the arguments
            uint16_t destAddr = strtol(args.c_str(), NULL, 16);
            // Call the appropriate method on the Monitoring singleton
            return Monitoring::getInstance().monitoringIdle(destAddr);
        }
    )); // End of first addCommand

    // Command: /monitoringActive <destination_address_hex>
    // Example usage: /monitoringActive A1B2
    addCommand(Command(
        "/monitoringActive", // Argument 1: Command name (const char*)
        "Set monitoring to Active. Arg: destination node hex addr (e.g., A1B2)", // Argument 2: Description (const char*)
        MonitoringState::mActive, // <<< FIX: Argument 3: State enum value (MonitoringState)
        1,                        // Argument 4: Number of arguments expected (int)
        [](String args) {         // Argument 5: Lambda function
            uint16_t destAddr = strtol(args.c_str(), NULL, 16);
            return Monitoring::getInstance().monitoringActive(destAddr);
        }
    )); // End of second addCommand

    // Add more commands here if needed following the same pattern
}