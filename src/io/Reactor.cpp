//
// Created by radeusgd on 27.05.18.
//

#include "Reactor.h"

#include <poll.h>
#include <utils/logging.h>
#include "utils/functional.h"

void Reactor::cleanIfEmpty(int fd) {
    auto wf = events.find(fd);
    if (!wf->second.in && !wf->second.out) {
        events.erase(wf);
    }
}

void Reactor::setOnReadable(int fd, std::function<void()> readable) {
    events[fd].in = std::move(readable);
}

void Reactor::setOnWriteable(int fd, std::function<void()> writeable) {
    events[fd].out = std::move(writeable);
}

void Reactor::cancelReading(int fd) {
    events[fd].in = nullptr;
    cleanIfEmpty(fd);
}

void Reactor::cancelWriting(int fd) {
    events[fd].out = nullptr;
    cleanIfEmpty(fd);
}

void Reactor::runAt(chrono::point t, std::function<void()> action) {
    timers.emplace(t, action);
}

void Reactor::runEvery(int ms, std::function<void()> action, RunEveryStartType start) {
    chrono::duration d = std::chrono::milliseconds(ms);
    auto f = [action, d, this](auto self) {
        action();
        runAt(chrono::now() + d, self);
    };

    auto t = chrono::now();
    if (start == RunEveryStartType::DEFER) {
        t += d;
    }
    runAt(t, simple_y_combinator{f});
}

void Reactor::run() {
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

        if (polls.empty()) {
            // TODO waiting for timers?
            return;
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
                dbg << "Some error happened on " << pfd.fd << "\n";
            }
            if (pfd.revents & POLLNVAL) {
                dbg << "Invalid request on " << pfd.fd << "!\n";
            }
            if (pfd.revents & POLLHUP) { // for example a pipe, doesn't call itself ready to read on EOF but calls this
                if (events[pfd.fd].in)
                    events[pfd.fd].in();
            }
        }
    }
}

int Reactor::timeout() {
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
