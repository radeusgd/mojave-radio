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
    std::string default_station_name;

    struct Station {
        std::string name;
        SockAddr addr;

        // sorting Stations lexicographically by name
        // for equal names, there's an arbitrary strict ordering
        bool operator<(const Station& o) const {
            if (name != o.name)
                return name < o.name;
            if (addr.sin_port != o.addr.sin_port)
                return addr.sin_port != o.addr.sin_port;
            return to_string(addr.sin_addr) < to_string(o.addr.sin_addr);
        }

        bool operator==(const Station& o) const {
            return name == o.name
                   && addr.sin_addr.s_addr == o.addr.sin_addr.s_addr
                   && addr.sin_port == o.addr.sin_port;
        }
    };

    using Stations = std::map<Station, chrono::point>;
    Stations stations; // mapping station to its last reply
    std::optional<Station> current_station = std::nullopt;

    void prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port);
    void prepareMenu();

    void handleReplyFrom(Station station);

    void moveMenuChoice(std::function<Stations::iterator(const Stations&, Stations::iterator)> mod);
    void stationListHasChanged();
    void refreshMenu();
public:
    Receiver(Reactor& reactor,
             IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
             size_t bsize, int rtime,
             std::string default_station_name);

    Receiver(const Receiver&) = delete;
};


#endif //MOJAVE_RADIO_RECEIVER_H
