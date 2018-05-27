//
// Created by radeusgd on 27.05.18.
//

#ifndef MOJAVE_RADIO_REACTOR_H
#define MOJAVE_RADIO_REACTOR_H

#include <utility>
#include <vector>
#include <functional>
#include <set>
#include <poll.h>
#include <cassert>
#include <map>

#include "chrono.h"
#include "functional.h"

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

    bool running;

    void cleanIfEmpty(int fd) {
        auto wf = events.find(fd);
        if (!wf->second.in && !wf->second.out) {
            events.erase(wf);
        }
    }

public:

    void setOnReadable(int fd, std::function<void()> readable) {
        events[fd].out = std::move(readable);
    }

    void setOnWriteable(int fd, std::function<void()> writeable) {
        events[fd].in = std::move(writeable);
    }

    void cancelReading(int fd) {
        events[fd].out = nullptr;
        cleanIfEmpty(fd);
    }

    void cancelWriting(int fd) {
        events[fd].in = nullptr;
        cleanIfEmpty(fd);
    }

    void runAt(chrono::point t, std::function<void()> action) {
        timers.emplace(t, action);
    }

    void runEvery(int ms, std::function<void()> action) {
        chrono::duration d = std::chrono::milliseconds(ms);
        auto f = [action, d, this](auto self) {
            action();
            runAt(chrono::now() + d, self);
        };
        runAt(chrono::now() + d, simple_y_combinator{f});
    }

    void run() {
        running = true;
        while (running) {
            std::vector<struct pollfd> polls;
            for (auto ev : events) {
                struct pollfd pfd{};
                pfd.fd = ev.first;
                pfd.events = 0;
                if (ev.second.in) {
                    pfd.events |= POLLIN;
                }
                if (ev.second.out) {
                    pfd.events |= POLLOUT;
                }
                polls.push_back(pfd);
            }

            int r = poll(&polls[0], polls.size(), timeout());
            while (running && hasPendingTimers()) { // while there are pending timers
                auto t = *timers.begin();
                timers.erase(timers.begin());
                t.action();
            }

            for (auto pfd : polls) {
                if (pfd.revents & POLLIN) {
                    events[pfd.fd].in();
                }
                if (pfd.revents & POLLOUT) {
                    events[pfd.fd].out();
                }
                if (pfd.revents & POLLERR) {
                    // TODO ?
                }
            }
        }
    }

    void stop() {
        running = false;
    }

private:
    bool hasPendingTimers() {
        return timeout() == 0;
    }

    int timeout() {
        if (timers.empty()) return -1;
        chrono::point soonest = timers.begin()->time;
        chrono::point now = chrono::now();

        int ms = chrono::to_millis(soonest - now);
        if (ms < 0) {
            // if somehow we missed an event, poll should return immediately
            return 0;
        }
        return ms;
    }
};


#endif //MOJAVE_RADIO_REACTOR_H
