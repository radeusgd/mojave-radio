//
// Created by radeusgd on 31.05.18.
//

#include "net/net.h"

#include <stdexcept>
#include <arpa/inet.h>

std::string IpAddr::to_string() const {
    char buff[INET_ADDRSTRLEN];
    std::string res = "[IP conversion error]";
    const char* r = inet_ntop(AF_INET, &underlying, buff, INET_ADDRSTRLEN);
    if (r != NULL) {
        res = buff;
    }
    return res;
}

IpAddr IpAddr::from_string(std::string str) {
    IpAddr ip;
    if (inet_pton(AF_INET, str.c_str(), &ip.underlying) != 1) {
        throw std::runtime_error(str + " is not a valid IP address");
    }
    return ip;
}

SockAddr::SockAddr(IpAddr ip, uint16_t port) {
    underlying.sin_family = AF_INET;
    underlying.sin_addr = ip.underlying;
    underlying.sin_port = htons(port);
}
