//
// Created by radeusgd on 31.05.18.
//

#ifndef MOJAVE_RADIO_TEXTMULTICASTSOCKET_H
#define MOJAVE_RADIO_TEXTMULTICASTSOCKET_H


#include "MulticastSocket.h"

class TextMulticastSocket : private MulticastSocket {
private:
    BytesBuffer strToData(const std::string& str) {
        return std::vector<char>(str.begin(), str.end());
    }
public:
    TextMulticastSocket(Reactor& reactor, IpAddr multicast_address)
        : MulticastSocket(reactor, multicast_address) {}

    void send(IpAddr destination, const std::string& message) {
        MulticastSocket::send(destination, strToData(message));
    }

    void broadcast(const std::string& message) {
        MulticastSocket::broadcast(strToData(message));
    }

    virtual void onReceived(IpAddr sender, const std::string& message) = 0;

    void onReceived(IpAddr sender, const BytesBuffer& data) override {
        if (data.empty() || data.back() != '\n') {
            // TODO malformed data
            return;
        }
        std::string str(data.begin(), std::prev(data.end()));
        onReceived(sender, str);
    }
};


#endif //MOJAVE_RADIO_TEXTMULTICASTSOCKET_H
