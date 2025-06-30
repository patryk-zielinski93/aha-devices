// Filename: PowerSocket.cpp
// Version: Refactored with Linked List, Two-Phase Init, and Memory Safety

#include "PowerSocket.h"

// Initialization of the static head pointer for the linked list
PowerSocket* PowerSocket::_head = nullptr;

// Constructor #1: Handles F() macro strings
PowerSocket::PowerSocket(
    HASwitch* haSwitch,
    const __FlashStringHelper* name,
    uint8_t switchPin,
    uint8_t switchedOnPinState,
    const __FlashStringHelper* icon
) : _haSwitch(haSwitch),
    _switchPin(switchPin),
    _switchedOnPinState(switchedOnPinState),
    _state(false),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _nextInstance(nullptr)
{
    _initialize(name, icon);
}

// Constructor #2: Handles standard C-strings
PowerSocket::PowerSocket(
    HASwitch* haSwitch,
    const char* name,
    uint8_t switchPin,
    uint8_t switchedOnPinState,
    const char* icon
) : _haSwitch(haSwitch),
    _switchPin(switchPin),
    _switchedOnPinState(switchedOnPinState),
    _state(false),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _nextInstance(nullptr)
{
    _initialize(name, icon);
}

// Destructor: Prevents memory leaks
PowerSocket::~PowerSocket()
{
    delete[] _haNameBuffer;
    delete[] _haIconBuffer;
}

// Private initializer for Flash strings
void PowerSocket::_initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon)
{
    // Allocate persistent buffers on the heap
    size_t nameLen = strlen_P(reinterpret_cast<const char*>(name));
    _haNameBuffer = new char[nameLen + 1];
    strcpy_P(_haNameBuffer, reinterpret_cast<const char*>(name));

    if (icon)
    {
        size_t iconLen = strlen_P(reinterpret_cast<const char*>(icon));
        _haIconBuffer = new char[iconLen + 1];
        strcpy_P(_haIconBuffer, reinterpret_cast<const char*>(icon));
    }

    // Call the common initializer
    _initialize(_haNameBuffer, _haIconBuffer);
}

// Central private initializer for RAM strings
void PowerSocket::_initialize(const char* name, const char* icon)
{
    // If coming from the SRAM constructor, make a persistent copy
    if (_haNameBuffer == nullptr)
    {
        _haNameBuffer = new char[strlen(name) + 1];
        strcpy(_haNameBuffer, name);
    }
    if (icon && _haIconBuffer == nullptr)
    {
        _haIconBuffer = new char[strlen(icon) + 1];
        strcpy(_haIconBuffer, icon);
    }

    // Add the newly created object to the front of the linked list
    _nextInstance = _head;
    _head = this;
}

// Static setup iterates through all instances
void PowerSocket::setup()
{
    for (PowerSocket* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_setup();
    }
}

// Instance setup configures hardware and callbacks
void PowerSocket::_setup()
{
    pinMode(_switchPin, OUTPUT);

    // Configure the HA entity
    _haSwitch->setName(_haNameBuffer);
    _haSwitch->setDeviceClass("outlet");
    if (_haIconBuffer)
    {
        _haSwitch->setIcon(_haIconBuffer);
    }

    // Register the static callback for HA commands
    _haSwitch->onCommand(PowerSocket::_onSwitchCommand);

    // Default to OFF state at startup, then update HA
    // Original code defaulted to ON, OFF is safer after a reboot.
    setState(false);
}

PowerSocket* PowerSocket::_findInstance(HASwitch* haSwitch)
{
    for (PowerSocket* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->_haSwitch == haSwitch)
        {
            return current;
        }
    }
    return nullptr;
}

void PowerSocket::_onSwitchCommand(bool state, HASwitch* sender)
{
    DPRINT(F("[PowerSocket: "));
    DPRINT(sender->uniqueId());
    DPRINT(F("] Received HA command: "));
    DPRINTLN(state);
    PowerSocket* socket = _findInstance(sender);
    if (socket != nullptr)
    {
        socket->setState(state);
    }
}

void PowerSocket::setState(bool state)
{
    // Use F() macro and multipart DPRINT to save SRAM
    DPRINT(F("[PowerSocket:"));
    DPRINT(_haSwitch->uniqueId());
    DPRINT(F("] setState("));
    DPRINT(state);
    DPRINTLN(F(")"));

    digitalWrite(_switchPin, state ? _switchedOnPinState : !_switchedOnPinState);
    _state = state;

    if (_haSwitch)
    {
        _haSwitch->setState(state, true); // Report state back to HA
    }
}

bool PowerSocket::getState() const
{
    return _state;
}

void PowerSocket::toggleState()
{
    setState(!_state);
}
