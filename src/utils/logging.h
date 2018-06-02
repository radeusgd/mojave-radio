//
// Created by radeusgd on 02.06.18.
//

#ifndef MOJAVE_RADIO_LOGGING_H
#define MOJAVE_RADIO_LOGGING_H

#ifdef DEBUG
#include <iostream>
#define dbg std::cerr
#else

#include <ostream>
class NoOpStream {};
extern NoOpStream dbg;
template<typename T> NoOpStream& operator<<(NoOpStream& nos, T t) {
    (void) t;
    return nos;
}
#endif

#endif //MOJAVE_RADIO_LOGGING_H
