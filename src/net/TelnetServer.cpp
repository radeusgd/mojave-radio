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

        buff.resize(r);
        for (char b : buff) {
            clientReceivedByte(c_sock, b);
        }
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

void TelnetServer::clientReceivedByte(int c_sock, unsigned char byte) {
    Client& c = clients[c_sock];
    using DM = Client::IncomingDataMode;
    constexpr unsigned char TELNET_ESCAPE = 255;
    // this encodes a state machine of Telnet protocol
    // we want to keep track of escape sequences and subnegotiations
    // but don't need their actual content, as we don't react to it in any way
    // we only extract arrow-key codes and act accordingly
    switch (c.mode) {
        case DM::NORMAL:
            if (byte == TELNET_ESCAPE) {
                c.mode = DM::ESCAPE;
            } else if (byte == 27) {
                c.mode = DM::VT1;
            } else {
                // we are not interested in normal characters being pressed
            }
            break;
        case DM::ESCAPE:
            if (byte == OP_SB) {
                // start subnegotiation
                c.mode = DM::SUBNEGOTIATION;
            } else if (251 <= byte && byte <= 254) {
                // handling special options
                c.mode = DM::OPTION;
            } else {
                // if byte == 255 - handle 255
                // but we don't handle raw bytes so doesn't matter
                c.mode = DM::NORMAL;
            }
            break;
        case DM::OPTION:
                c.mode = DM::NORMAL;
            break;
        case DM::SUBNEGOTIATION:
            // we actually don't process what's inside subnegotiation
            if (byte == TELNET_ESCAPE) {
                c.mode = DM::SUBNEGOTIATION_ESCAPE;
            }
            break;
        case DM::SUBNEGOTIATION_ESCAPE:
            if (byte == OP_SE) {
                c.mode = DM::NORMAL;
            } else {
                // we're not interested in other escape values
            }
            break;
        case DM::VT1:
            if (byte == 91) {
                c.mode = DM::VT1_SUB;
            } else {
                // we are not interested in such escape sequence so go back to normal
                c.mode = DM::NORMAL;
            }
            break;
        case DM::VT1_SUB:
            c.mode = DM::NORMAL;
            unsigned char keys[] = {KEY_UP, KEY_DOWN};
            for (unsigned char k : keys) {
                if (byte == k) {
                    input_handler(static_cast<InputKey>(k));
                }
            }
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

void TelnetServer::setInputHandler(std::function<void(InputKey)> handler) {
    input_handler = handler;
}

