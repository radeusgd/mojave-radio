//
// Created by radeusgd on 04.06.18.
//

#ifndef MOJAVE_RADIO_RECEIVER_H
#define MOJAVE_RADIO_RECEIVER_H


#include <net/net.h>
#include <io/Reactor.h>
#include <net/TextUDPSocket.h>

class Receiver {
private:
    Reactor& reactor;
    TextUDPSocket discovery_sock;

    void prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port);
    // TODO
public:
    Receiver(Reactor& reactor,
             IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
             size_t bsize, int rtime);

    Receiver(const Receiver&) = delete;
};


#endif //MOJAVE_RADIO_RECEIVER_H
