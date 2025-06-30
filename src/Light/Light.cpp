#include "Light.h"

// Initialization of the static head pointer for our linked list.
Light* Light::_head = nullptr;

// Constructor #1: Handles F() macro strings (__FlashStringHelper*)
Light::Light(
    uint8_t pin,
    HALight* haLight,
    const __FlashStringHelper* name,
    const __FlashStringHelper* icon
) : _pin(pin),
    _currentState(false),
    _haLight(haLight),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _nextInstance(nullptr) {
    _initialize(name, icon);
}

// Constructor #2: Handles standard C-strings (const char*)
Light::Light(
    uint8_t pin,
    HALight* haLight,
    const char* name,
    const char* icon
) : _pin(pin),
    _currentState(false),
    _haLight(haLight),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _nextInstance(nullptr) {
    _initialize(name, icon);
}

// Destructor: This is critical to prevent memory leaks.
// It frees the memory we allocated on the heap in the constructor.
Light::~Light() {
    delete[] _haNameBuffer;
    delete[] _haIconBuffer;
}

// Private initializer for Flash strings.
void Light::_initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon) {
    // Allocate a persistent buffer on the heap for the name
    if (name) {
        size_t nameLen = strlen_P(reinterpret_cast<const char*>(name));
        _haNameBuffer = new char[nameLen + 1];
        strcpy_P(_haNameBuffer, reinterpret_cast<const char*>(name));
    }

    // Do the same for the icon, if provided
    if (icon) {
        size_t iconLen = strlen_P(reinterpret_cast<const char*>(icon));
        _haIconBuffer = new char[iconLen + 1];
        strcpy_P(_haIconBuffer, reinterpret_cast<const char*>(icon));
    }

    // Call the original (private) initialize method with the new RAM pointers.
    _initialize(_haNameBuffer, _haIconBuffer);
}

// Private initializer for RAM strings.
// This is now the central point for all instance setup logic.
void Light::_initialize(const char* name, const char* icon) {
    // If we are coming from the SRAM string constructor, we still make a copy
    // to keep ownership of the memory within the class.
    if (name && _haNameBuffer == nullptr) {
        _haNameBuffer = new char[strlen(name) + 1];
        strcpy(_haNameBuffer, name);
    }
    if (icon && _haIconBuffer == nullptr) {
        _haIconBuffer = new char[strlen(icon) + 1];
        strcpy(_haIconBuffer, icon);
    }

    // Configure the HA entity with our persistent RAM pointers
    _haLight->setName(_haNameBuffer);
    if (icon) {
        _haLight->setIcon(_haIconBuffer);
    }

    // Add the newly created object to the front of the linked list
    _nextInstance = _head;
    _head = this;
}


// The static setup() method iterates through the linked list
// and calls the private _setup() on each registered instance.
void Light::setup() {
    DPRINTLN(F("Setting up all Light instances..."));
    for (Light* current = _head; current != nullptr; current = current->_nextInstance) {
        current->_setup();
    }
}

// The private _setup() method handles hardware and callback configuration.
void Light::_setup() {
    DPRINT(F("-> Configuring Light: "));
    DPRINTLN(_haLight->uniqueId());
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);

    // Register the static callback function now, at a safe time.
    _haLight->onStateCommand(Light::onStateCommand);
}


// The rest of the implementation is for finding instances and handling callbacks.
Light* Light::findInstance(HALight* haLight) {
    for (Light* current = _head; current != nullptr; current = current->_nextInstance) {
        if (current->_haLight == haLight) {
            return current;
        }
    }
    return nullptr;
}

void Light::onStateCommand(bool state, HALight* sender) {
    DPRINT(F("[Light] Received HA command: "));
    DPRINTLN(state);
    if (Light* light = findInstance(sender); light != nullptr) {
        light->setState(state);
    }
}

void Light::setState(bool state) {
    if (state == _currentState) return;

    DPRINT(F("[Light:"));
    DPRINT(_haLight->uniqueId());
    DPRINT(F("] setState("));
    DPRINT(state);
    DPRINTLN(F(")"));

    digitalWrite(_pin, state ? HIGH : LOW);
    _currentState = state;

    if (_haLight) {
        _haLight->setState(state, true);
    }
}

void Light::toggleState() {
    setState(!_currentState);
}

void Light::turnOn() {
    setState(true);
}

void Light::turnOff() {
    setState(false);
}

bool Light::getState() const {
    return _currentState;
}
