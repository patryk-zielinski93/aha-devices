#ifndef AHA_DEVICES_COVER_H
#define AHA_DEVICES_COVER_H

#include <Arduino.h>
#include <ArduinoHA.h>
#include "Debug.h"
#include "Button/Button.h"

/**
 * @class Cover
 * @brief Controls a motorized cover (blind, shutter, gate) based on run time.
 * This class implements a state machine for positioning and tilt, supports HA integration,
 * and uses a linked list for safe, dynamic instance management.
 */
class Cover
{
public:
    // --- Static methods to control all created instances ---
    static void setup();
    static void loop();
    static void closeAll();
    static void openAll();
    static void openCover(Cover* cover, ButtonEvent event);
    static void closeCover(Cover* cover, ButtonEvent event);

    // --- Public constants ---
    inline static int calibrationTimeMs = 1000;

    // --- Constructors ---
    // Overloaded constructor for F() macro (Flash strings)
    Cover(
        HACover* haCover,
        const __FlashStringHelper* name,
        uint8_t motorDownPin,
        uint8_t motorUpPin,
        long fullCourseTimeMs,
        long fullCourseTiltTimeMs,
        const char* deviceClass = nullptr,
        const __FlashStringHelper* icon = nullptr
    );

    // Overloaded constructor for standard C-strings (RAM)
    Cover(
        HACover* haCover,
        const char* name,
        uint8_t motorDownPin,
        uint8_t motorUpPin,
        long fullCourseTimeMs,
        long fullCourseTiltTimeMs,
        const char* deviceClass = nullptr,
        const char* icon = nullptr
    );

    // Destructor to free dynamically allocated memory
    virtual ~Cover();

    // --- Public API for controlling a single instance ---
    void setTargetPosition(uint8_t position);
    void setTargetTiltPosition(uint8_t position);
    void stop();
    void open();
    void close();
    uint8_t getCurrentPosition() const;
    uint8_t getCurrentTilt() const;
    bool isTargeting() const;

private:
    // Internal states for the state machine
    enum CoverState { StateIdle, StateTargetingPosition, StateTargetingTilt };

    enum MotorDirection { DirectionNone, DirectionUp, DirectionDown };

    // --- Private member variables ---
    HACover* _haCover;
    uint8_t _motorDownPin;
    uint8_t _motorUpPin;

    long _fullCourseTimeMs;
    long _fullCourseTiltTimeMs;
    long _currentPositionMs = 0;
    long _currentTiltPositionMs = 0;
    long _targetPositionMs = 0;
    long _targetTiltPositionMs = 0;
    unsigned long _lastUpdatedAt = 0;
    unsigned long _safetyDelayEnd = 0;

    CoverState _state = StateIdle;
    MotorDirection _motorState = DirectionNone;
    bool _tiltEnabled;

    // Buffers for heap-allocated name/icon strings
    char* _haNameBuffer;
    char* _haIconBuffer;

    // --- Private instance methods (prefixed with _) ---
    void _initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon, const char* deviceClass);
    void _initialize(const char* name, const char* icon, const char* deviceClass);
    void _setup() const;
    void _loop();
    void _motorUp();
    void _motorDown();
    void _motorStop();
    void _stateIdle();
    void _stateTargetingPosition();
    void _stateTargetingTilt();
    void _startSafetyDelay();
    bool _isSafetyDelayActive();
    void _updateHAState() const;

    // --- Linked List for Instance Management ---
    Cover* _nextInstance;
    static Cover* _head;

    // --- Private static callback handling (no prefix) ---
    static Cover* findInstance(HACover* haCover);
    static void onPositionCommand(uint8_t position, HACover* sender);
    static void onTiltCommand(uint8_t tilt, HACover* sender);
    static void onCommand(HACover::CoverCommand command, HACover* sender);
};

#endif //AHA_DEVICES_COVER_H
