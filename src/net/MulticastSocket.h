//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_MULTICASTSERVER_H
#define MOJAVE_RADIO_MULTICASTSERVER_H

#include <io/io.h>
#include <io/Reactor.h>
#include <deque>
#include "net.h"

static constexpr size_t BUFSIZE = 1024;

/*
 * A server that can receive and send packets to/from a multicast address
 * or reply to just one of the senders.
 */
class MulticastSocket {
    int sock;
    SockAddr multicast_address;
    struct PendingMessage {
        SockAddr destination;
        BytesBuffer data;
        PendingMessage(SockAddr destination, BytesBuffer data)
            : destination(destination), data(std::move(data)) {}
    };
    std::deque<PendingMessage> send_queue;

    Reactor& reactor;

    void registerWriter();
public:
    MulticastSocket(Reactor& reactor, SockAddr multicast_address);

    void send(SockAddr destination, const BytesBuffer& data);
    void send(SockAddr destination, BytesBuffer&& data);

    void broadcast(const BytesBuffer& data);
    void broadcast(BytesBuffer&& data);

    virtual void onReceived(SockAddr sender, const BytesBuffer& data) = 0;
};


#endif //MOJAVE_RADIO_MULTICASTSERVER_H
