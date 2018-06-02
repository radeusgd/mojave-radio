//
// Created by radeusgd on 28.05.18.
//

#include "MulticastSocket.h"

#include <unistd.h>
#include <utility>
#include "utils/errors.h"

MulticastSocket::MulticastSocket(Reactor &reactor, SockAddr multicast_address, MulticastMode mode)
    : reactor(reactor),
      multicast_address(multicast_address) {
    // initialize socket
    sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if (sock < 0) {
        raise_errno("sock");
    }

    // enable broadcast
    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
        raise_errno("setsockopt broadcast");
    }

    // TODO
    //optval = DEFAULT_TTL;
    //setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &optval, sizeof(optval))
    optval = 1; // TODO this is mostly for testing, dunno
    if (setsockopt(sock, SOL_IP, IP_MULTICAST_LOOP, &optval, sizeof(optval)) < 0) {
        raise_errno("setsockopt loop");
    }

    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        raise_errno("setsockopt reuseaddr");
    }

    if (mode == MulticastMode::REGISTER_TO_MULTICAST_GROUP) {
        // register to multicast group
        struct ip_mreq ip_mreq{};
        ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        ip_mreq.imr_multiaddr = multicast_address.sin_addr;
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ip_mreq, sizeof(struct ip_mreq)) < 0) {
            raise_errno("setsockopt add_membership");
        }
    }

    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (mode == MulticastMode::REGISTER_TO_MULTICAST_GROUP) {
        addr.sin_port = multicast_address.sin_port;
    } else {
        addr.sin_port = htons(0); // if we are not listening to multicast (only broadcasting), we can have any port
    }
    if (bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0) {
        raise_errno("bind");
    }

    // register reader
    reactor.setOnReadable(sock, [&reactor, this]() {
        std::vector<char> buffer;
        buffer.resize(BUFSIZE);
        SockAddr src_addr;
        socklen_t len = sizeof(src_addr);
        ssize_t r = recvfrom(sock, &buffer[0], BUFSIZE, MSG_DONTWAIT,
                             reinterpret_cast<sockaddr *>(&src_addr), &len);
        if (r < 0) {
            raise_errno("sock read");
            return;
        }

        if (r == 0) {
            throw std::runtime_error("UDP shouldn't close");
        } else {
            buffer.resize(r);
            receive_hook(src_addr, buffer);
        }
    });
}

void MulticastSocket::registerWriter() {
    reactor.setOnWriteable(sock, [this]() {
        assert (!send_queue.empty());
        PendingMessage &pm = send_queue.front();
        auto r = sendto(sock, &pm.data[0], pm.data.size(),
                        0, reinterpret_cast<const sockaddr *>(&pm.destination),
                        sizeof(pm.destination));
        if (r == 0)
            return;
        if (r < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            raise_errno("write"); // TODO? EAGAIN?
        }
        if (r != pm.data.size()) {
            throw std::runtime_error("Partial datagram write?");
        }

        // message successfully written
        if (pm.callback) pm.callback();
        send_queue.pop_front();

        // if we have no more packets to write, unregister
        if (send_queue.empty()) {
            reactor.cancelWriting(sock);
        }
    });
}

void MulticastSocket::send(SockAddr destination, const BytesBuffer &data, std::function<void()> callback) {
    send_queue.emplace_back(destination, data, std::move(callback));
    if (send_queue.size() == 1) {
        // we are enqueuing the first packet, register the handler
        registerWriter();
    }
}

void MulticastSocket::send(SockAddr destination, BytesBuffer &&data, std::function<void()> callback) {
    send_queue.emplace_back(destination, std::move(data), std::move(callback));
    if (send_queue.size() == 1) {
        // we are enqueuing the first packet, register the handler
        registerWriter();
    }
}

void MulticastSocket::broadcast(const BytesBuffer &data, std::function<void()> callback) {
    send(multicast_address, data, std::move(callback));
}

void MulticastSocket::broadcast(BytesBuffer &&data, std::function<void()> callback) {
    send(multicast_address, std::move(data), std::move(callback));
}

void MulticastSocket::setOnReceived(MulticastSocket::OnReceive hook) {
    receive_hook = std::move(hook);
}

MulticastSocket::~MulticastSocket() {
    reactor.cancelReading(sock);
    reactor.cancelWriting(sock);
    close(sock);
}
