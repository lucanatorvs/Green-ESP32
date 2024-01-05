#ifndef MY_TIMERS_H
#define MY_TIMERS_H

#include <Arduino.h>
#include <functional>
#include <vector>

class Timer {
public:
    Timer(unsigned long timeout, std::function<void()> callback, bool isWatchdog = false);
    ~Timer();

    void start();
    void stop();
    void reset();
    bool isRunning() const;

    static void timerTask(void *pvParameters);

private:
    unsigned long _timeout;
    bool _isRunning;
    bool _isWatchdog;
    std::function<void()> _callback;
    unsigned long _lastResetTime;

    static std::vector<Timer*> _timers;
};

void initializeTimerTask();

#endif // MY_TIMERS_H