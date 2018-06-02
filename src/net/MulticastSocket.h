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

enum class MulticastMode {
    REGISTER_TO_MULTICAST_GROUP, // this mode registers to the multicast group and listens for messages sent there
    MULTICAST_SEND_ONLY // this mode listens on a normal port and is just allowed to send messages to the multicast address
};

/*
 * A server that can receive and send packets to/from a multicast address
 * or reply to just one of the senders.
 */
class MulticastSocket {
public:
    using OnReceive = std::function<void(SockAddr, const BytesBuffer&)>;
private:
    int sock;
    SockAddr multicast_address;
    struct PendingMessage {
        SockAddr destination;
        BytesBuffer data;
        std::function<void()> callback;
        PendingMessage(SockAddr destination, BytesBuffer data, std::function<void()> callback)
            : destination(destination), data(std::move(data)), callback(std::move(callback)) {}
    };
    std::deque<PendingMessage> send_queue;

    Reactor& reactor;
    OnReceive receive_hook;

    void registerWriter();
public:
    MulticastSocket(Reactor& reactor, SockAddr multicast_address, MulticastMode mode);

    void send(SockAddr destination, const BytesBuffer& data, std::function<void()> callback = nullptr);
    void send(SockAddr destination, BytesBuffer&& data, std::function<void()> callback = nullptr);

    void broadcast(const BytesBuffer& data, std::function<void()> callback = nullptr);
    void broadcast(BytesBuffer&& data, std::function<void()> callback = nullptr);

    void setOnReceived(OnReceive hook);

    ~MulticastSocket();
};


#endif //MOJAVE_RADIO_MULTICASTSERVER_H
