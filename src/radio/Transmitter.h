//
// Created by radeusgd on 02.06.18.
//

#ifndef MOJAVE_RADIO_NADAJNIK_H
#define MOJAVE_RADIO_NADAJNIK_H


#include <io/Reactor.h>
#include <net/MulticastSocket.h>
#include <net/TextMulticastSocket.h>
#include <utils/logging.h>
#include <sstream>
#include <io/StdinReader.h>
#include "protocol.h"

class Transmitter {
private:
    Reactor& reactor;
    TextMulticastSocket ctrl_sock;
    MulticastSocket data_sock;

    std::string LOOKUP_REPLY;

    void sendReply(SockAddr destination);

public:
    Transmitter(Reactor& reactor,
             IpAddr multicast_addr, uint16_t data_port, uint16_t ctrl_port,
             size_t psize, size_t fsize, int rtime,
             const std::string &name);
};


#endif //MOJAVE_RADIO_NADAJNIK_H
