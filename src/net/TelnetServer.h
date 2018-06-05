//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_TCPSERVER_H
#define MOJAVE_RADIO_TCPSERVER_H

#include <io/Reactor.h>
#include <map>
#include <io/io.h>
#include <deque>

class TelnetServer {
public:
    enum InputKey {
        KEY_UP = 'A',
        KEY_DOWN = 'B'
    };
private:
    enum OptMode {
        OP_SE = 240,
        OP_SB = 250,
        OP_WILL = 251,
        OP_WONT = 252,
        OP_DO = 253,
        OP_DONT = 254
    };
    enum Option {
        OPT_ECHO = 1,
        OPT_SUPPRESS_GO_AHEAD = 3,
        OPT_LINEMODE = 34
    };

    Reactor& reactor;
    int sock;
    struct Client {
        int sock;
        std::deque<BytesBuffer> queue; // TODO transient messages
        size_t current_bytes_sent = 0;
        enum class IncomingDataMode {
            NORMAL,
            ESCAPE,
            OPTION,
            SUBNEGOTIATION,
            SUBNEGOTIATION_ESCAPE,
            VT1,
            VT1_SUB
        } mode = IncomingDataMode::NORMAL;
    };

    void clientConnected(int c_sock);
    void clientDisconnected(int c_sock);
    void clientReceivedByte(int c_sock, unsigned char byte);
    std::map<int, Client> clients; // sock -> Client

    void send(int c_sock, const BytesBuffer& data);
    void sendOption(int c_sock, OptMode mode, Option opt);

    BytesBuffer current_contents;

    std::function<void(InputKey)> input_handler;
public:
    TelnetServer(Reactor& reactor, uint16_t listen_port);

    void setInputHandler(std::function<void(InputKey)> handler);
    void setWindowContents(std::string contents);

    ~TelnetServer();
};


#endif //MOJAVE_RADIO_TCPSERVER_H
