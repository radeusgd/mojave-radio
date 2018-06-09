//
// Created by radeusgd on 28.05.18.
//

#include "UDPSocket.h"
#include "utils/errors.h"

#include <unistd.h>
#include <utility>
#include <sys/ioctl.h>

static constexpr size_t BUFFSIZE = 64 * 1024 + 1;

UDPSocket::UDPSocket(Reactor &reactor, uint16_t port)
    : reactor(reactor) {
    bind(port);
}

void UDPSocket::unbind() {
    if (sock == -1)
        return;
    reactor.cancelReading(sock);
    reactor.cancelWriting(sock);
    close(sock);
    sock = -1;
}

void UDPSocket::bind(uint16_t port) {
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

    optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        raise_errno("setsockopt reuseaddr");
    }

    // bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (::bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0) {
        raise_errno("bind");
    }

    // register reader
    reactor.setOnReadable(sock, [this]() {
        std::vector<char> buffer;

        buffer.resize(BUFFSIZE);
        SockAddr src_addr;
        socklen_t len = sizeof(src_addr.underlying);
        ssize_t r = recvfrom(sock, &buffer[0], BUFFSIZE, MSG_DONTWAIT,
                             reinterpret_cast<sockaddr *>(&src_addr.underlying), &len);
        if (r < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                return;
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

    reactor.markDirty();
}

void UDPSocket::registerWriter() {
    reactor.setOnWriteable(sock, [this]() {
        assert (!send_queue.empty());
        PendingMessage &pm = send_queue.front();
        ssize_t r = sendto(sock, &pm.data[0], pm.data.size(),
                        0, reinterpret_cast<const sockaddr *>(&pm.destination.underlying),
                        sizeof(pm.destination.underlying));
        if (r == 0)
            return;
        if (r < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            raise_errno("write");
        }

        if (static_cast<size_t >(r) != pm.data.size()) {
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

void UDPSocket::registerToMulticastGroup(IpAddr addr) {
    struct ip_mreq ip_mreq{};
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ip_mreq.imr_multiaddr = addr.underlying;
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ip_mreq, sizeof(struct ip_mreq)) < 0) {
        raise_errno("setsockopt add_membership");
    }

    reactor.markDirty();
}

void UDPSocket::unregisterFromMulticastGroup(IpAddr addr) {
    struct ip_mreq ip_mreq{};
    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ip_mreq.imr_multiaddr = addr.underlying;
    if (setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &ip_mreq, sizeof(struct ip_mreq)) < 0) {
        raise_errno("setsockopt drop_membership");
    }

    reactor.markDirty();
}

void UDPSocket::send(SockAddr destination, const BytesBuffer &data, std::function<void()> callback) {
    send_queue.emplace_back(destination, data, std::move(callback));
    if (send_queue.size() == 1) {
        // we are enqueuing the first packet, register the handler
        registerWriter();
    }
}

void UDPSocket::send(SockAddr destination, BytesBuffer &&data, std::function<void()> callback) {
    send_queue.emplace_back(destination, std::move(data), std::move(callback));
    if (send_queue.size() == 1) {
        // we are enqueuing the first packet, register the handler
        registerWriter();
    }
}

void UDPSocket::setOnReceived(UDPSocket::OnReceive hook) {
    receive_hook = std::move(hook);
}

UDPSocket::~UDPSocket() {
    unbind();
}

void UDPSocket::rebind(uint16_t new_port) {
    unbind();
    bind(new_port);
}

