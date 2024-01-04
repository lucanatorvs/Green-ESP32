#ifndef TIMERS_H
#define TIMERS_H

#include <Arduino.h>

class Timer {
public:
    Timer(unsigned long timeout, std::function<void()> callback, bool isWatchdog = false);

    void start();
    void stop();
    void reset();
    bool isRunning() const;

private:
    unsigned long _timeout;
    bool _isRunning;
    bool _isWatchdog;
    std::function<void()> _callback;
    unsigned long _lastResetTime;

    void _checkTimer();
};


#endif // TIMERS_H