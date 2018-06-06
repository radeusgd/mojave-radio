//
// Created by radeusgd on 27.05.18.
//

#ifndef MOJAVE_RADIO_REACTOR_H
#define MOJAVE_RADIO_REACTOR_H

#include <iostream>
#include <vector>
#include <functional>
#include <set>
#include <cassert>
#include <map>

#include "utils/chrono.h"

class Reactor {
private:
    struct TimedAction {
        chrono::point time;
        std::function<void()> action;

        TimedAction(chrono::point t, std::function<void()> a)
            : time(t), action(std::move(a)) {}

        bool operator<(const TimedAction& o) const {
            return time < o.time;
        }
    };

    struct WaitFor {
        std::function<void()> in;
        std::function<void()> out;
    };

    std::map<int, WaitFor> events;
    std::multiset<TimedAction> timers;

    bool running = false;
    bool dirty = false;

    void cleanIfEmpty(int fd);

public:
    Reactor(const Reactor&) = delete;
    Reactor() = default;

    void setOnReadable(int fd, std::function<void()> readable);

    void setOnWriteable(int fd, std::function<void()> writeable);

    void cancelReading(int fd);

    void cancelWriting(int fd);

    void runAt(chrono::point t, std::function<void()> action);

    enum class RunEveryStartType {
        START_NOW,
        DEFER
    };
    void runEvery(int ms, std::function<void()> action, RunEveryStartType start = RunEveryStartType::DEFER);

    void run();

    /*
     * When some operation makes it possible for some file descriptor
     * that could have been marked ready by poll to be no longer ready
     * (closing a socket, changing listen address etc.)
     * it should call markDirty to make sure an invalidated callback doesn't get called.
     */
    void markDirty() {
        dirty = true;
    }

    void stop() {
        running = false;
    }

private:
    bool hasPendingTimers() {
        return timeout() == 0;
    }

    int timeout();
};


#endif //MOJAVE_RADIO_REACTOR_H
