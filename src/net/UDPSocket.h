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
 * An UDP socket.
 */
class UDPSocket {
public:
    using OnReceive = std::function<void(SockAddr, const BytesBuffer&)>;
private:
    int sock;
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
    UDPSocket(Reactor& reactor, uint16_t bind_port = 0);

    void registerToMulticastGroup(IpAddr addr);
    void unregisterFromMulticastGroup(IpAddr addr);

    void send(SockAddr destination, const BytesBuffer& data, std::function<void()> callback = nullptr);
    void send(SockAddr destination, BytesBuffer&& data, std::function<void()> callback = nullptr);

    void setOnReceived(OnReceive hook);

    ~UDPSocket();
};


#endif //MOJAVE_RADIO_MULTICASTSERVER_H
