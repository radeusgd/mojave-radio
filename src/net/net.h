//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_NET_H
#define MOJAVE_RADIO_NET_H

#include <netinet/ip.h>
#include <string>

using IpAddr = struct sockaddr_in;

constexpr int DEFAULT_TTL = 4;

IpAddr ipaddr_from_string(std::string str);

#endif //MOJAVE_RADIO_NET_H
