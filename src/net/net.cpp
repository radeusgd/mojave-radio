//
// Created by radeusgd on 31.05.18.
//

#include "net/net.h"

#include <stdexcept>
#include <arpa/inet.h>

IpAddr ipaddr_from_string(std::string str) {
    auto colon = str.find(':');
    if (colon == std::string::npos) {
        throw std::runtime_error("wrong ip format");
    }
    std::string ip_part = str.substr(0, colon);
    std::string port_part = str.substr(colon + 1);

    IpAddr ip;
    ip.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip_part.c_str(), &(ip.sin_addr)) != 1) {
        throw std::runtime_error(ip_part + " is not a valid IP address");
    }
    auto port = atoi(port_part.c_str()); // TODO error handling
    if (port == 0 || port > 65535) { // TODO
        throw std::runtime_error("port out of range");
    }

    ip.sin_port = htons(static_cast<uint16_t>(port));

    return ip;
}