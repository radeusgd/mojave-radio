//
// Created by radeusgd on 27.05.18.
//

#ifndef MOJAVE_RADIO_TIME_H
#define MOJAVE_RADIO_TIME_H

#include <chrono>

namespace chrono {
    using point = std::chrono::steady_clock::time_point;
    using duration = std::chrono::steady_clock::duration;

    inline int to_millis(duration d) {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(d);
        return static_cast<int>(diff.count()); // TODO
    }

    inline point now() {
        return std::chrono::steady_clock::now();
    }
}

#endif //MOJAVE_RADIO_TIME_H
