//
// Created by radeusgd on 28.05.18.
//

#include <sys/socket.h>
#include <utils/errors.h>
#include <netinet/in.h>
#include <unistd.h>
#include <regex>
#include <utils/logging.h>
#include "TelnetServer.h"

static const std::string CLEAR_SCREEN = "\u001B[2J";
static const std::string MOVE_HOME = "\u001B[H";

TelnetServer::TelnetServer(Reactor &reactor, uint16_t listen_port)
    : reactor(reactor) {
    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock < 0) {
        raise_errno("sock");
    }

    int optval = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        raise_errno("setsockopt reuseaddr");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(listen_port);
    if (bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0) {
        raise_errno("bind");
    }

    if (listen(sock, 128) == -1) {
        raise_errno("listen");
    }

    reactor.setOnReadable(sock, [&reactor, this]() {
        int s = accept(sock, NULL, NULL);
        if (s == -1) {
            raise_errno("accept");
        }
        clientConnected(s);
    });
}

void TelnetServer::setWindowContents(std::string contents) {
    // add special bytes and convert line endings
    contents = CLEAR_SCREEN + MOVE_HOME + std::regex_replace(contents, std::regex("\n"), "\n\r");
    current_contents = {contents.begin(), contents.end()};
    for (auto c : clients) {
        send(c.second.sock, current_contents); // TODO transient
    }
}

void TelnetServer::clientConnected(int c_sock) {
    dbg << "Client connected.\n";
    Client c;
    c.sock = c_sock;
    clients.emplace(c_sock, c);

    reactor.setOnReadable(c_sock, [this, c_sock]() {
        BytesBuffer buff;
        buff.resize(100); // TODO
        ssize_t r = recv(c_sock, &buff[0], buff.size(), MSG_DONTWAIT);
        if (r < 0) {
            raise_errno("tcp recv");
        }

        if (r == 0) {
            clientDisconnected(c_sock);
            return;
        }

        dbg << "Recvd " << r << " bytes\n";
    });

    sendOption(c_sock, OP_DONT, OPT_LINEMODE); // TODO is it needed?
    sendOption(c_sock, OP_WILL, OPT_ECHO);
    sendOption(c_sock, OP_WILL, OPT_SUPPRESS_GO_AHEAD);
    send(c_sock, current_contents);
}


void TelnetServer::clientDisconnected(int c_sock) {
    dbg << "Client disconnected.\n";
    clients.erase(c_sock);
    reactor.cancelWriting(c_sock);
    reactor.cancelReading(c_sock);
    if (close(c_sock) == -1) {
        raise_errno("close");
    }
}

void TelnetServer::send(int c_sock, const BytesBuffer& data) {
    clients[c_sock].queue.emplace_back(data);
    if (clients[c_sock].queue.size() == 1) { // first packet in queue - schedule writing
        reactor.setOnWriteable(c_sock, [this, c_sock]() {
            auto& client = clients[c_sock];
            BytesBuffer &b = client.queue.front();
            ssize_t sent = ::send(c_sock, &b[client.current_bytes_sent], b.size() - client.current_bytes_sent, MSG_DONTWAIT);
            if (sent < 0) {
                raise_errno("tcp send");
            }
            client.current_bytes_sent += sent;

            if (client.current_bytes_sent == b.size()) {
                client.queue.pop_front();
                client.current_bytes_sent = 0;
                if (client.queue.empty()) {
                    // if there's no more to send for now
                    reactor.cancelWriting(c_sock);
                }
            }
        });
    }
}

void TelnetServer::sendOption(int c_sock, OptMode mode, Option opt) {
    BytesBuffer msg;
    msg.push_back(static_cast<char>(255)); // special control char
    msg.push_back(static_cast<char>(mode));
    msg.push_back(static_cast<char>(opt));
    send(c_sock, msg);
}

TelnetServer::~TelnetServer() {
    for (auto c : clients) {
        reactor.cancelWriting(c.first);
        reactor.cancelReading(c.first);
        close(c.first);
    }
    reactor.cancelReading(sock);
    close(sock);
}

