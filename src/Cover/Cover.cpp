#include "Cover.h"

// Initialization of the static head pointer for our linked list.
Cover* Cover::_head = nullptr;

// --- Constructors and Destructor ---

Cover::Cover(
    HACover* haCover,
    const __FlashStringHelper* name,
    uint8_t motorDownPin,
    uint8_t motorUpPin,
    long fullCourseTimeMs,
    // --- EEPROM Configuration ---
    uint16_t eepromAddrPosition,
    uint8_t eepromSlotsPosition,
    // --- Optional Tilt Configuration ---
    long fullCourseTiltTimeMs,
    uint16_t eepromAddrTilt,
    uint8_t eepromSlotsTilt,
    // --- Optional HA Configuration ---
    const char* deviceClass,
    const __FlashStringHelper* icon
) : _haCover(haCover),
    _motorDownPin(motorDownPin),
    _motorUpPin(motorUpPin),
    _fullCourseTimeMs(fullCourseTimeMs),
    _fullCourseTiltTimeMs(fullCourseTiltTimeMs),
    _eepromAddrPosition(eepromAddrPosition),
    _eepromSlotsPosition(eepromSlotsPosition),
    _eepromAddrTilt(eepromAddrTilt),
    _eepromSlotsTilt(eepromSlotsTilt),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _nextInstance(nullptr)
{
    _initialize(name, icon, deviceClass);
}

Cover::Cover(
    HACover* haCover,
    const char* name,
    uint8_t motorDownPin,
    uint8_t motorUpPin,
    long fullCourseTimeMs,
    // --- EEPROM Configuration ---
    uint16_t eepromAddrPosition,
    uint8_t eepromSlotsPosition,
    // --- Optional Tilt Configuration ---
    long fullCourseTiltTimeMs,
    uint16_t eepromAddrTilt,
    uint8_t eepromSlotsTilt,
    // --- Optional HA Configuration ---
    const char* deviceClass,
    const char* icon
) : _haCover(haCover),
    _motorDownPin(motorDownPin),
    _motorUpPin(motorUpPin),
    _fullCourseTimeMs(fullCourseTimeMs),
    _fullCourseTiltTimeMs(fullCourseTiltTimeMs),
    _eepromAddrPosition(eepromAddrPosition),
    _eepromSlotsPosition(eepromSlotsPosition),
    _eepromAddrTilt(eepromAddrTilt),
    _eepromSlotsTilt(eepromSlotsTilt),
    _haNameBuffer(nullptr),
    _haIconBuffer(nullptr),
    _nextInstance(nullptr)
{
    _initialize(name, icon, deviceClass);
}

Cover::~Cover()
{
    delete[] _haNameBuffer;
    delete[] _haIconBuffer;
}

// --- Initializers ---

void Cover::_initialize(const __FlashStringHelper* name, const __FlashStringHelper* icon, const char* deviceClass)
{
    if (name)
    {
        size_t nameLen = strlen_P(reinterpret_cast<const char*>(name));
        _haNameBuffer = new char[nameLen + 1];
        strcpy_P(_haNameBuffer, reinterpret_cast<const char*>(name));
    }

    if (icon)
    {
        size_t iconLen = strlen_P(reinterpret_cast<const char*>(icon));
        _haIconBuffer = new char[iconLen + 1];
        strcpy_P(_haIconBuffer, reinterpret_cast<const char*>(icon));
    }

    _initialize(_haNameBuffer, _haIconBuffer, deviceClass);
}

void Cover::_initialize(const char* name, const char* icon, const char* deviceClass)
{
    if (name && !_haNameBuffer)
    {
        _haNameBuffer = new char[strlen(name) + 1];
        strcpy(_haNameBuffer, name);
    }
    if (icon && !_haIconBuffer)
    {
        _haIconBuffer = new char[strlen(icon) + 1];
        strcpy(_haIconBuffer, icon);
    }

    if (_haNameBuffer) _haCover->setName(_haNameBuffer);
    if (deviceClass) _haCover->setDeviceClass(deviceClass);
    if (_haIconBuffer) _haCover->setIcon(_haIconBuffer);

    _tiltEnabled = (_fullCourseTiltTimeMs != 0);
    _nextInstance = _head;
    _head = this;
}

// --- Static Methods ---

void Cover::setup()
{
    for (Cover* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_setup();
    }
}

void Cover::loop()
{
    for (Cover* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->_loop();
    }
}

void Cover::closeAll()
{
    for (Cover* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->close();
    }
}

void Cover::openAll()
{
    for (Cover* current = _head; current != nullptr; current = current->_nextInstance)
    {
        current->open();
    }
}

void Cover::openCover(Cover* cover, ButtonEvent event)
{
    if (!cover) return;
    switch (event)
    {
    case BUTTON_EVENT_CLICKED:
        if (cover->isTargeting())
        {
            cover->stop();
        }
        else
        {
            cover->open();
        }
        break;

    case BUTTON_EVENT_PRESSED:

        if (cover->_motorState == DirectionDown)
        {
            cover->stop();
        }
        cover->open();
        break;

    case BUTTON_EVENT_RELEASED:
        cover->stop();
        break;

    case BUTTON_EVENT_LONG_PRESSED:
        break;
    }
}

void Cover::closeCover(Cover* cover, ButtonEvent event)
{
    if (!cover) return;
    switch (event)
    {
    case BUTTON_EVENT_CLICKED:
        if (cover->isTargeting())
        {
            cover->stop();
        }
        else
        {
            cover->close();
        }
        break;

    case BUTTON_EVENT_PRESSED:
        if (cover->_motorState == DirectionUp)
        {
            cover->stop();
        }

        cover->close();
        break;

    case BUTTON_EVENT_RELEASED:
        cover->stop();
        break;
    }
}

// --- Static Callback Handlers ---

Cover* Cover::findInstance(HACover* haCover)
{
    for (Cover* current = _head; current != nullptr; current = current->_nextInstance)
    {
        if (current->_haCover == haCover)
        {
            return current;
        }
    }
    return nullptr;
}

void Cover::onPositionCommand(uint8_t position, HACover* sender)
{
    DPRINT(F("[Cover] #_onPositionCommand: "));
    DPRINTLN(position);
    Cover* cover = findInstance(sender);

    if (cover == nullptr)
    {
        return;
    }

    if (cover->isTargeting())
    {
        cover->_motorStop();
        cover->_state = StateIdle;
    }

    if (cover->getCurrentPosition() < position)
    {
        DPRINTLN(F("[Cover] #_onPositionCommand -> setTilt: 0"));
        cover->setTargetTiltPosition(0);
    }
    else if (cover->getCurrentPosition() > position)
    {
        DPRINTLN(F("[Cover] #_onPositionCommand -> setTilt 100"));
        cover->setTargetTiltPosition(100);
    }

    cover->_updateHAState();
    cover->setTargetPosition(100 - position);
    DPRINT(F("[Cover] #_onPositionCommand -> setTargetPosition: "));
    DPRINTLN(cover->_targetPositionMs);
}

void Cover::onTiltCommand(uint8_t tilt, HACover* sender)
{
    DPRINT(F("[Cover] #_onTiltCommand: "));
    DPRINTLN(tilt);
    Cover* cover = findInstance(sender);

    if (cover == nullptr || !cover->_tiltEnabled)
    {
        return;
    }

    cover->_updateHAState();
    cover->setTargetTiltPosition(100 - tilt);
}

void Cover::onCommand(HACover::CoverCommand command, HACover* sender)
{
    DPRINT(F("[Cover] #_onCommand: "));
    DPRINTLN(command);
    Cover* cover = findInstance(sender);

    if (cover == nullptr)
    {
        return;
    }

    cover->_updateHAState();

    switch (command)
    {
    case HACover::CoverCommand::CommandStop:
        cover->stop();
        break;
    case HACover::CoverCommand::CommandOpen:
        cover->open();
        break;
    case HACover::CoverCommand::CommandClose:
        cover->close();
        break;
    }
}

void Cover::_setup()
{
    _currentPositionMs = _targetPositionMs = EepromService::read<long>(_eepromAddrPosition);
    if (_tiltEnabled)
    {
        _currentTiltPositionMs = _targetTiltPositionMs = EepromService::read<long>(_eepromAddrTilt);
    }

    pinMode(_motorUpPin, OUTPUT);
    pinMode(_motorDownPin, OUTPUT);
    digitalWrite(_motorUpPin, LOW);
    digitalWrite(_motorDownPin, LOW);

    _haCover->onCommand(onCommand);
    _haCover->onSetPositionCommand(onPositionCommand);
    _haCover->onTiltCommand(onTiltCommand);
    _haCover->setRetain(true);
    _haCover->setOptimistic(true);

    _updateHAState();
}

void Cover::_loop()
{
    switch (_state)
    {
    case StateIdle:
        _stateIdle();
        break;
    case StateTargetingPosition:
        _stateTargetingPosition();
        break;
    case StateTargetingTilt:
        _stateTargetingTilt();
        break;
    }
}

void Cover::_motorUp()
{
    DPRINTLN(F("[Cover] #_motorUp()"));
    digitalWrite(_motorUpPin, HIGH);
    digitalWrite(_motorDownPin, LOW);
    _motorState = DirectionUp;
}

void Cover::_motorDown()
{
    DPRINTLN(F("[Cover] #_motorDown()"));
    digitalWrite(_motorUpPin, LOW);
    digitalWrite(_motorDownPin, HIGH);
    _motorState = DirectionDown;
}

void Cover::_motorStop()
{
    DPRINTLN(F("[Cover] #_motorStop()"));
    digitalWrite(_motorUpPin, LOW);
    digitalWrite(_motorDownPin, LOW);
    _motorState = DirectionNone;
    _startSafetyDelay();
    _updateHAState();
}

void Cover::_stateIdle()
{
    if (_currentPositionMs != _targetPositionMs)
    {
        _state = StateTargetingPosition;
        DPRINTLN(F("[Cover] #_stateIdle() -> StateTargetingPosition"));
        return;
    }

    if (_currentTiltPositionMs != _targetTiltPositionMs)
    {
        _state = StateTargetingTilt;
        DPRINTLN(F("[Cover] #_stateIdle() -> StateTargetingTilt"));
        return;
    }
}

void Cover::_stateTargetingPosition()
{
    if (_isSafetyDelayActive())
    {
        // DPRINTLN("[Cover] #_stateTargeting() -> Safety delay active");
        return;
    }

    if (MotorDirection::DirectionNone == _motorState)
    {
        DPRINTLN(F("[Cover] #_stateTargeting() -> _motorState=DirectionNone"));
        DPRINT(F("[Cover] #_stateTargeting() -> _currentPositionMs: "));
        DPRINTLN(_currentPositionMs);
        DPRINT(F("[Cover] #_stateTargeting() -> _targetPositionMs: "));
        DPRINTLN(_targetPositionMs);

        if (_currentPositionMs > _targetPositionMs)
        {
            DPRINT(F("[Cover] #_stateTargeting() -> motorUp -> _currentPositionMs: "));
            DPRINTLN(_currentPositionMs);
            _haCover->setState(HACover::CoverState::StateOpening, true);
            _motorUp();
        }
        else if (_currentPositionMs < _targetPositionMs)
        {
            DPRINT(F("[Cover] #_stateTargeting() -> motorDown -> _currentPositionMs: "));
            DPRINTLN(_currentPositionMs);
            _haCover->setState(HACover::CoverState::StateClosing, true);
            _motorDown();
        }
        else if (_currentPositionMs == _targetPositionMs)
        {
            DPRINT(F("[Cover] #_stateTargeting() -> _state=StateIdle -> _currentPositionMs: "));
            DPRINTLN(_currentPositionMs);
            _state = _tiltEnabled ? StateTargetingTilt : StateIdle;
        }

        _lastUpdatedAt = millis();
        return;
    }

    if (MotorDirection::DirectionDown == _motorState)
    {
        uint32_t change = millis() - _lastUpdatedAt;
        _lastUpdatedAt = millis();

        if (_tiltEnabled && _currentTiltPositionMs < _fullCourseTiltTimeMs)
        {
            _currentTiltPositionMs += change;

            if (_currentTiltPositionMs > _fullCourseTiltTimeMs)
            {
                _currentTiltPositionMs = _fullCourseTiltTimeMs;
            }
        }

        _currentPositionMs += change;

        if (_currentPositionMs >= _targetPositionMs)
        {
            if (_currentPositionMs >= _fullCourseTimeMs && _currentPositionMs - _fullCourseTimeMs < calibrationTimeMs)
            {
                // calibration on each full course
                DPRINTLN(F("[Cover] #_stateTargeting() -> calibration active"));
                return;
            }

            if (_currentPositionMs > _fullCourseTimeMs)
            {
                _currentPositionMs = _fullCourseTimeMs;
            }

            stop();
            DPRINTLN(F("[Cover] #_stateTargeting() direction down -> _motorStop()"));
        }

        return;
    }

    if (MotorDirection::DirectionUp == _motorState)
    {
        uint32_t change = millis() - _lastUpdatedAt;
        _lastUpdatedAt = millis();

        _currentPositionMs -= change;

        if (_tiltEnabled && _currentTiltPositionMs > 0)
        {
            _currentTiltPositionMs -= change;
            if (_currentTiltPositionMs < 0)
            {
                _currentTiltPositionMs = 0;
            }
        }

        if (_currentPositionMs <= _targetPositionMs)
        {
            if (_currentPositionMs <= 0 && _currentPositionMs > -calibrationTimeMs)
            {
                // calibration on each full course
                DPRINTLN(F("[Cover] #_stateTargeting() -> calibration active"));
                return;
            }

            if (_currentPositionMs < 0)
            {
                _currentPositionMs = 0;
            }

            stop();
            DPRINTLN(F("[Cover] #_stateTargeting() direction up -> _motorStop()"));
        }
    }
}

void Cover::_stateTargetingTilt()
{
    if (_isSafetyDelayActive())
    {
        _lastUpdatedAt = millis();
        DPRINTLN(F("[Cover] #_stateTargeting() -> Safety delay active"));
        return;
    }

    if (MotorDirection::DirectionNone == _motorState)
    {
        DPRINTLN(F("[Cover] #_stateTargetingTilt() -> _motorState=DirectionNone"));
        DPRINT(F("[Cover] #_stateTargetingTilt() -> _currentTiltPositionMs: " ));
        DPRINTLN(_currentTiltPositionMs);
        DPRINT(F("[Cover] #_stateTargetingTilt() -> _targetTiltPositionMs: "));
        DPRINTLN(_targetTiltPositionMs);
        if (_currentTiltPositionMs > _targetTiltPositionMs)
        {
            _motorUp();
        }
        else if (_currentTiltPositionMs < _targetTiltPositionMs)
        {
            _motorDown();
        }
        else if (_currentTiltPositionMs == _targetTiltPositionMs)
        {
            _state = StateIdle;
            DPRINTLN(F("[Cover] #_stateTargetingTilt() -> _state=StateIdle"));
        }

        _lastUpdatedAt = millis();
        return;
    }

    if (MotorDirection::DirectionDown == _motorState)
    {
        uint32_t change = millis() - _lastUpdatedAt;
        _lastUpdatedAt = millis();
        _currentTiltPositionMs += change;

        if (_currentPositionMs < _fullCourseTimeMs)
        {
            _currentPositionMs += change;
            if (_currentPositionMs > _fullCourseTimeMs)
            {
                _currentPositionMs = _fullCourseTimeMs;
            }
        }

        if (_currentTiltPositionMs >= _targetTiltPositionMs)
        {
            if (_currentTiltPositionMs > _fullCourseTiltTimeMs)
            {
                _currentTiltPositionMs = _fullCourseTiltTimeMs;
            }

            stop();
            DPRINTLN(F("[Cover] #_stateTargetingTilt() directionDown -> _motorStop()"));
        }

        return;
    }

    if (MotorDirection::DirectionUp == _motorState)
    {
        uint32_t change = millis() - _lastUpdatedAt;
        _lastUpdatedAt = millis();

        _currentPositionMs -= change;

        if (change >= _currentTiltPositionMs)
        {
            _currentTiltPositionMs = 0;
        }
        else
        {
            _currentTiltPositionMs -= change;
        }

        if (_currentTiltPositionMs <= _targetTiltPositionMs)
        {
            stop();
            DPRINTLN(F("[Cover] #_stateTargetingTilt() directionUp -> _motorStop()"));
        }
    }
}

bool Cover::_isSafetyDelayActive()
{
    return millis() < _safetyDelayEnd;
}

void Cover::_startSafetyDelay()
{
    _safetyDelayEnd = millis() + 250;
}

void Cover::setTargetPosition(uint8_t position)
{
    _targetPositionMs = map(position, 0, 100, 0, _fullCourseTimeMs);
    DPRINT(F("[Cover] #setTargetPosition("));
    DPRINT(position);
    DPRINT(F(") -> _targetPositionMs: "));
    DPRINTLN(_targetPositionMs);

    DPRINT(F("[Cover] #setTargetPosition -> position: "));
    DPRINT(position);
    DPRINT(F(", _fullCourseTimeMs: "));
    DPRINTLN(_fullCourseTimeMs);
}

void Cover::setTargetTiltPosition(uint8_t position)
{
    _targetTiltPositionMs = map(position, 0, 100, 0, _fullCourseTiltTimeMs);
    DPRINT(F("[Cover] #setTargetTiltPosition("));
    DPRINT(position);
    DPRINT(F(") -> _targetTiltPositionMs: "));
    DPRINTLN(_targetTiltPositionMs);
}

void Cover::stop()
{
    _targetPositionMs = _currentPositionMs;
    _targetTiltPositionMs = _currentTiltPositionMs;
    _motorStop();
    DPRINT(F("[Cover] #stop() -> _targetPositionMs: "));
    DPRINT(_targetPositionMs);
    DPRINTLN(F(" _motorStop()"));
    _state = Cover::StateIdle;
    EepromService::write<long>(_eepromAddrPosition, _currentPositionMs);
    if (_tiltEnabled)
    {
        EepromService::write<long>(_eepromAddrTilt, _currentTiltPositionMs);
    }
}

void Cover::open()
{
    if (_tiltEnabled)
    {
        setTargetTiltPosition(0);
    }
    setTargetPosition(0);
}

void Cover::close()
{
    if (_tiltEnabled)
    {
        setTargetTiltPosition(100);
    }
    setTargetPosition(100);
}

uint8_t Cover::getCurrentTilt() const
{
    return 100 - 100 * _currentTiltPositionMs / _fullCourseTiltTimeMs;
}

uint8_t Cover::getCurrentPosition() const
{
    return 100 - 100 * _currentPositionMs / _fullCourseTimeMs;
}

bool Cover::isTargeting() const
{
    return _state == StateTargetingPosition || _state == StateTargetingTilt;
}

void Cover::_updateHAState() const
{
    if (_currentTiltPositionMs == _targetTiltPositionMs && _currentPositionMs == _targetPositionMs)
    {
        _haCover->setPosition(getCurrentPosition(), true);
        DPRINT(F("[Cover] #_updateHAState() -> currentPosition: "));
        DPRINT(getCurrentPosition());
        DPRINT(F(" (current position ms: "));
        DPRINT(_currentPositionMs);
        DPRINT(F(" full course time ms: "));
        DPRINT(_fullCourseTimeMs);
        DPRINTLN(F(")"));

        if (_tiltEnabled)
        {
            _haCover->setTilt(getCurrentTilt(), true);
            DPRINT(F("[Cover] #_updateHAState() -> currentTilt: "));
            DPRINT(getCurrentTilt());
            DPRINT(F(" "));
            DPRINTLN(_currentTiltPositionMs);
        }

        if (
            _currentPositionMs == _fullCourseTimeMs &&
            (!_tiltEnabled ? true : _currentTiltPositionMs == _fullCourseTiltTimeMs)
        )
        {
            DPRINTLN(F("[Cover] #_updateHAState() -> currentState: StateClosed"));
            _haCover->setState(HACover::CoverState::StateClosed, true);
        }
        else
        {
            DPRINTLN(F("[Cover] #_updateHAState() -> currentState: StateOpen"));
            _haCover->setState(HACover::CoverState::StateOpen, true);
        }
    }
}
