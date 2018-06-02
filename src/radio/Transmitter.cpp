//
// Created by radeusgd on 02.06.18.
//

#include "Transmitter.h"

void Transmitter::sendReply(SockAddr destination) {
    ctrl_sock.send(destination, LOOKUP_REPLY);
}

Transmitter::Transmitter(Reactor &reactor,
                         IpAddr multicast_addr, uint16_t data_port, uint16_t ctrl_port,
                         size_t psize, size_t fsize, int rtime,
                         const std::string &name)
    : reactor(reactor),
      ctrl_sock(reactor, make_sockaddr(multicast_addr, ctrl_port), MulticastMode::REGISTER_TO_MULTICAST_GROUP),
      data_sock(reactor, make_sockaddr(multicast_addr, data_port), MulticastMode::MULTICAST_SEND_ONLY)
{

    {
        std::stringstream ss;
        ss << REPLY << " " << multicast_addr << " " << data_port << " " << name;
        LOOKUP_REPLY = ss.str();
    }

    data_sock.setOnReceived([](SockAddr source, const BytesBuffer& data) {});

    ctrl_sock.setOnReceived([this](SockAddr source, const std::string& message) {
        if (message == LOOKUP) {
            dbg << "Lookup request from " << source << "\n";
            sendReply(source);
        } else if (message == REXMIT) {
            dbg << "Retransmission request from " << source << "\n";
        } else {
            dbg << "Unknown message: " << message << "\n";
        }
    });

    read_from_stdin(reactor, [this](BytesBuffer data) {
        // TODO
    }, [&reactor]() {
        dbg << "End of stdin\n";
        reactor.stop();
    });
}
