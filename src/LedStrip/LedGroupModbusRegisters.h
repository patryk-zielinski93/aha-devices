//
// Created by zielq on 23.07.2025.
//

#ifndef LEDGROUPMODBUSREGISTERS_H
#define LEDGROUPMODBUSREGISTERS_H
#include <Arduino.h>

// Number of holding registers per LED strip group
constexpr uint8_t REGS_PER_GROUP = 12;

// Register map for each group
enum ModbusLedGroupRegisters
{
    REG_STATE = 0, // Slave reports its current state here
    // Registers written by Master
    REG_COMMAND = 1, // Master writes commands here
    REG_MODE = 2, // 0: Static color, 1: Animation
    REG_BRIGHTNESS = 3, // 0-255
    REG_R = 4, // 0-255
    REG_G = 5, // 0-255
    REG_B = 6, // 0-255
    REG_CW = 7, // 0-255 Cold white
    REG_WW = 8, // 0-255 Warm white
    REG_ANIMATION_ID = 9, // Animation ID to run
    REG_ANIMATION_SPEED = 10, // Animation speed (1-255)
    REG_COLOR_CYCLE = 11 // 0: Off, 1: On (for animations)
};

enum LedGroupMode
{
    MODE_STATIC = 0, // Static mode
    MODE_ANIMATION = 1 // Animation mode
};

enum LedGroupState
{
    STATE_IDLE_OFF = 0, // State: Off, idle. Also command confirming turn off.
    STATE_READY_TO_TURN_ON = 1, // Status: Ready to turn on (relay can operate)
    STATE_TURNING_ON = 2,
    STATE_ON = 3, // State: On, normal operation
    STATE_TURNING_OFF = 4, // State: Turning off
    STATE_READY_TO_TURN_OFF = 5 // Status: Ready to turn off (relay can operate)
};

enum LedGroupCommand
{
    CMD_IDLE = 0, // Command: Off / idle
    CMD_PREPARE_TURN_OFF = 1, // Command: Prepare to turn off
    CMD_TURN_OFF = 2, // Command: Off
    CMD_PREPARE_TURN_ON = 3, // Command: Prepare to turn on
    CMD_TURN_ON = 4 // Command: Relay has been turned on, turn on strip
};

#endif //LEDGROUPMODBUSREGISTERS_H
