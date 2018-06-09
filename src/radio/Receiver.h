//
// Created by radeusgd on 04.06.18.
//

#ifndef MOJAVE_RADIO_RECEIVER_H
#define MOJAVE_RADIO_RECEIVER_H

#include "net/net.h"
#include "io/Reactor.h"
#include "net/TextUDPSocket.h"
#include "net/TelnetServer.h"
#include "io/StdoutWriter.h"
#include "IncomingAudioBuffer.h"
#include "protocol.h"
#include <map>

class Receiver {
private:
    static constexpr int DISCOVER_PERIOD_MS = 5 * 1000;
    static constexpr int DISCOVER_TIMEOUT = 20 * 1000;

    Reactor& reactor;
    TextUDPSocket discovery_sock;
    UDPSocket audio_sock;
    StdoutWriter stdout_writer;
    TelnetServer ui_server;
    std::string default_station_name;

    struct Station {
        std::string name;
        SockAddr audio_mcast_addr;
        SockAddr rexmit_addr;

        // sorting Stations lexicographically by name
        // for equal names, there's an arbitrary strict ordering
        bool operator<(const Station& o) const {
            if (name != o.name)
                return name < o.name;
            if (audio_mcast_addr != o.audio_mcast_addr)
                return audio_mcast_addr < o.audio_mcast_addr;
            return rexmit_addr < o.rexmit_addr;
        }

        bool operator==(const Station& o) const {
            return name == o.name
                   && audio_mcast_addr == o.audio_mcast_addr
                   && rexmit_addr == o.rexmit_addr;
        }

        bool operator!=(const Station& o) const {
            return !(*this == o);
        }
    };

    using Stations = std::map<Station, chrono::point>;
    Stations stations; // mapping station to its last reply
    std::optional<Station> current_station = std::nullopt;
    struct Session {
        uint64_t id;
        size_t psize;
        uint64_t byte0;

        uint64_t byte_to_start_bursting;
        uint64_t latest_received_pkg_id; // used for REXMITs

        std::optional<uint64_t> bursting_pkg_id;
    };
    std::optional<Session> session = std::nullopt;
    uint64_t bsize;
    int rexmit_time;
    IncomingAudioBuffer buffer;

    void resetSession();

    void prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port);
    void prepareMenu();
    void prepareAudio();

    void handleReplyFrom(Station station);

    void moveMenuChoice(std::function<Stations::iterator(const Stations&, Stations::iterator)> mod);
    void stationListHasChanged();
    void changeCurrentStation(Station new_station);
    void refreshMenu();

    void startListening(Station station);
    void stopListening(Station station);
    void handleIncomingPackage(AudioPackage&& pkg);

    void writeToStdout();

    void scheduleRexmitRequest(std::set<uint64_t> missing_ids, uint64_t session_id);
    void sendRexmitRequest(std::set<uint64_t> missing_ids);
public:
    Receiver(Reactor& reactor,
             IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
             size_t bsize, int rtime,
             std::string default_station_name);

    Receiver(const Receiver&) = delete;
};


#endif //MOJAVE_RADIO_RECEIVER_H
