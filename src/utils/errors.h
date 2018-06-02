//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_ERRORS_H
#define MOJAVE_RADIO_ERRORS_H

#include <string>
#include <stdexcept>

class c_system_error : public std::runtime_error {
public:
    explicit c_system_error(const std::string &msg)
        : std::runtime_error(msg) {}
};

/*
 * Throws an exception with a message and errno.
 */
void raise_errno(std::string msg);

#endif //MOJAVE_RADIO_ERRORS_H
