//
// Created by radeusgd on 04.06.18.
//

#include "Receiver.h"
#include "protocol.h"

Receiver::Receiver(Reactor& reactor,
                   IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
                   size_t bsize, int rtime)
    : reactor(reactor),
      discovery_sock(reactor)
{
    prepareDiscovery(discover_addr, ctrl_port);
}

void Receiver::prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port) {
    SockAddr discovery_broadcast_addr = make_sockaddr(discover_addr, ctrl_port);

    discovery_sock.setOnReceived([this](SockAddr source, const std::string& message) {
        std::smatch smatch;
        if (std::regex_match(message, smatch, REPLY_REGEX)) {
            dbg << "TODO parse reply " << message << "\n";
        } else {
            dbg << "Unknown message: " << message << "\n";
        }
    });

    reactor.runEvery(5000, [this, discovery_broadcast_addr]() {
        discovery_sock.send(discovery_broadcast_addr, LOOKUP);
    }, Reactor::RunEveryStartType::START_NOW);
}
