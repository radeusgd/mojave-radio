//
// Created by radeusgd on 27.05.18.
//

#ifndef MOJAVE_RADIO_FUNCTIONAL_H
#define MOJAVE_RADIO_FUNCTIONAL_H

#include <functional>
#include <utility>

class simple_y_combinator {
private:
    std::function<void(std::function<void()>)> f;

public:
    explicit simple_y_combinator(std::function<void(std::function<void()>)> f)
        : f(std::move(f)) {}

    void operator()() const {
        std::function<void()> self = *this;
        f(self);
    }
};

#endif //MOJAVE_RADIO_FUNCTIONAL_H
