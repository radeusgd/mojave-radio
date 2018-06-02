//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_NET_H
#define MOJAVE_RADIO_NET_H

#include <netinet/ip.h>
#include <string>
#include <ostream>

using SockAddr = struct sockaddr_in;
using IpAddr = struct in_addr;

constexpr int DEFAULT_TTL = 4;

IpAddr ipaddr_from_string(std::string str);

SockAddr make_sockaddr(IpAddr ip, uint16_t port);

std::string to_string(IpAddr ip);

inline std::ostream& operator<<(std::ostream& os, IpAddr ip) {
    return os << to_string(ip);
}

inline std::ostream& operator<<(std::ostream& os, SockAddr addr) {
    return os << addr.sin_addr << ":" << addr.sin_port;
}

#endif //MOJAVE_RADIO_NET_H
