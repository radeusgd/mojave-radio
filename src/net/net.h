//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_NET_H
#define MOJAVE_RADIO_NET_H

#include <netinet/ip.h>
#include <string>
#include <ostream>

#define MAX_PORT_NUMBER 65535

class IpAddr {
public:
    using Underlying = struct in_addr;
    Underlying underlying{};
    IpAddr() = default;

    explicit IpAddr(struct in_addr addr) : underlying(addr) {}

    std::string to_string() const;

    bool operator==(const IpAddr& o) const {
        return underlying.s_addr == o.underlying.s_addr;
    }

    static IpAddr from_string(std::string str);
};

struct SockAddr {
public:
    using Underlying = struct sockaddr_in;
    Underlying underlying{};
    SockAddr() = default;

    explicit SockAddr(struct sockaddr_in addr) : underlying(addr) {}
    SockAddr(IpAddr ip, uint16_t port);

    inline IpAddr ip() const {
        return IpAddr(underlying.sin_addr);
    }

    inline uint16_t port() const {
        return ntohs(underlying.sin_port);
    }

    bool operator==(const SockAddr& o) const {
        return ip() == o.ip() && port() == o.port();
    }
};

inline std::ostream& operator<<(std::ostream& os, IpAddr ip) {
    return os << ip.to_string();
}

inline std::ostream& operator<<(std::ostream& os, SockAddr addr) {
    return os << addr.ip() << ":" << addr.port();
}

#endif //MOJAVE_RADIO_NET_H
