#include "Timers.h"

std::vector<Timer*> Timer::_timers;

Timer::Timer(unsigned long timeout, std::function<void()> callback, bool isWatchdog)
    : _timeout(timeout), _callback(callback), _isWatchdog(isWatchdog), _isRunning(false), _lastResetTime(0) {
    _timers.push_back(this);
}

Timer::~Timer() {
    _timers.erase(std::remove(_timers.begin(), _timers.end(), this), _timers.end());
}

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

void Timer::timerTask(void *pvParameters) {
    for (;;) {
        for (auto& timer : _timers) {
            // Check and update each timer
            if (timer->_isRunning) {
                unsigned long currentTime = millis();
                if ((currentTime - timer->_lastResetTime) >= timer->_timeout) {
                    timer->_callback();
                    if (!timer->_isWatchdog) {
                        timer->_isRunning = false;
                    } else {
                        timer->_lastResetTime = currentTime;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay for a short while to prevent task from using all CPU time
    }
}

void initializeTimerTask() {
    xTaskCreate(Timer::timerTask, "TimerTask", 2048, nullptr, 3, nullptr);
}