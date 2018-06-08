//
// Created by radeusgd on 31.05.18.
//

#ifndef MOJAVE_RADIO_TEXTMULTICASTSOCKET_H
#define MOJAVE_RADIO_TEXTMULTICASTSOCKET_H


#include "UDPSocket.h"
#include "utils/logging.h"
#include <utility>

class TextUDPSocket {
public:
    using OnReceive = std::function<void(SockAddr, const std::string&)>;
private:
    BytesBuffer strToData(const std::string& str) {
        std::vector<char> res(str.begin(), str.end());
        res.push_back('\n');
        return res;
    }

    UDPSocket underlying;
    OnReceive receive_hook;
public:
    explicit TextUDPSocket(Reactor &reactor, uint16_t port = 0)
        : underlying(reactor, port) {
        underlying.setOnReceived([this](SockAddr source, const BytesBuffer &data) {
            if (data.empty() || data.back() != '\n') {
                dbg << "Text socket received a malformed message\n";
                return;
            }
            std::string str(data.begin(), std::prev(data.end())); // trime end-of-line
            receive_hook(source, str);
        });
    }

    void registerToMulticastGroup(IpAddr addr) {
        underlying.registerToMulticastGroup(addr);
    }

    void unregisterFromMulticastGroup(IpAddr addr) {
        underlying.unregisterFromMulticastGroup(addr);
    }

    void send(SockAddr destination, const std::string& message) {
        underlying.send(destination, strToData(message));
    }

    void setOnReceived(OnReceive hook) {
        receive_hook = std::move(hook);
    }
};


#endif //MOJAVE_RADIO_TEXTMULTICASTSOCKET_H
