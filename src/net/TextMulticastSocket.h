//
// Created by radeusgd on 31.05.18.
//

#ifndef MOJAVE_RADIO_TEXTMULTICASTSOCKET_H
#define MOJAVE_RADIO_TEXTMULTICASTSOCKET_H


#include "MulticastSocket.h"
#include <utility>
#include <utils/logging.h>

class TextMulticastSocket {
public:
    using OnReceive = std::function<void(SockAddr, const std::string&)>;
private:
    BytesBuffer strToData(const std::string& str) {
        std::vector<char> res(str.begin(), str.end());
        res.push_back('\n');
        return res;
    }

    MulticastSocket underlying;
    OnReceive receive_hook;
public:
    TextMulticastSocket(Reactor &reactor, SockAddr multicast_address, MulticastMode mode)
        : underlying(reactor, multicast_address, mode) {
        underlying.setOnReceived([this](SockAddr source, const BytesBuffer &data) {
            if (data.empty() || data.back() != '\n') {
                dbg << "Text socket received a strange message\n";
                return;
            }
            std::string str(data.begin(), std::prev(data.end())); // trime end-of-line
            receive_hook(source, str);
        });
    }

    void send(SockAddr destination, const std::string& message) {
        underlying.send(destination, strToData(message));
    }

    void broadcast(const std::string& message) {
        underlying.broadcast(strToData(message));
    }

    void setOnReceived(OnReceive hook) {
        receive_hook = std::move(hook);
    }
};


#endif //MOJAVE_RADIO_TEXTMULTICASTSOCKET_H
