//
// Created by radeusgd on 28.05.18.
//

#include <cerrno>
#include <cstring>
#include <sstream>

#include "errors.h"

void raise_errno(std::string msg) {
    std::stringstream ss;
    ss << "System error in " << msg
       << " (" << errno << "; " << strerror(errno) << ")\n";

    throw c_system_error{ss.str()};
}

