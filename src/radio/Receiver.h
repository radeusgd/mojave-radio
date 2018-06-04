//
// Created by radeusgd on 04.06.18.
//

#ifndef MOJAVE_RADIO_RECEIVER_H
#define MOJAVE_RADIO_RECEIVER_H


#include <net/net.h>
#include <io/Reactor.h>
#include <net/TextUDPSocket.h>
#include <map>
#include <net/TelnetServer.h>

class Receiver {
private:
    static constexpr int DISCOVER_PERIOD_MS = 5 * 1000;
    static constexpr int DISCOVER_TIMEOUT = 20 * 1000;

    Reactor& reactor;
    TextUDPSocket discovery_sock;
    TelnetServer ui_server;

    struct Station {
        std::string name;
        SockAddr addr;

        // some arbitrary strong order on Stations
        bool operator<(const Station& o) const {
            if (name != o.name)
                return name < o.name;
            if (addr.sin_port != o.addr.sin_port)
                return addr.sin_port != o.addr.sin_port;
            return to_string(addr.sin_addr) < to_string(o.addr.sin_addr);
        }
    };
    std::map<Station, chrono::point> stations; // mapping station to its last reply

    void prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port);

    void handleReplyFrom(Station station);
    // TODO
    void stationListHasChanged();

    void refreshMenu();
public:
    Receiver(Reactor& reactor,
             IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
             size_t bsize, int rtime);

    Receiver(const Receiver&) = delete;
};


#endif //MOJAVE_RADIO_RECEIVER_H
