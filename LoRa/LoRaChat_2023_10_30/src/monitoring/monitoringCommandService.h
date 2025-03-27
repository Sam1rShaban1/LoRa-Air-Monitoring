// monitoringCommandService.h
#pragma once

#include "Arduino.h"
#include "commands/commandService.h" // Include the base class definition

/**
 * @brief CommandService implementation specifically for Monitoring commands.
 *        Registers commands like /monitoringIdle and /monitoringActive.
 */
class MonitoringCommandService: public CommandService {
public:
    /**
     * @brief Constructor that adds the monitoring-specific commands.
     */
    MonitoringCommandService();
};