#include "Timers.h"

Timer::Timer(unsigned long timeout, std::function<void()> callback, bool isWatchdog) 
    : _timeout(timeout), _callback(callback), _isWatchdog(isWatchdog), _isRunning(false), _lastResetTime(0) {}

void Timer::start() {
    _isRunning = true;
    _lastResetTime = millis();
}

void Timer::stop() {
    _isRunning = false;
}

void Timer::reset() {
    _lastResetTime = millis();
}

bool Timer::isRunning() const {
    return _isRunning;
}

void Timer::_checkTimer() {
    if (!_isRunning) return;

    unsigned long currentTime = millis();
    if ((currentTime - _lastResetTime >= _timeout)) {
        _callback();
        if (!_isWatchdog) {
            _isRunning = false;
        } else {
            _lastResetTime = currentTime;
        }
    }
}