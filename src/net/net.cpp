//
// Created by radeusgd on 31.05.18.
//

#include "net/net.h"

#include <stdexcept>
#include <arpa/inet.h>

IpAddr ipaddr_from_string(std::string str) {
    IpAddr ip;
    if (inet_pton(AF_INET, str.c_str(), &ip) != 1) {
        throw std::runtime_error(str + " is not a valid IP address");
    }
    return ip;
}

SockAddr make_sockaddr(IpAddr ip, uint16_t port) {
    SockAddr addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = ip;
    addr.sin_port = htons(port);
    return addr;
}
