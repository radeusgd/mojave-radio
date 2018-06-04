//
// Created by radeusgd on 04.06.18.
//

#include <utils/string.h>
#include "Receiver.h"
#include "protocol.h"

Receiver::Receiver(Reactor& reactor,
                   IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
                   size_t bsize, int rtime)
    : reactor(reactor),
      discovery_sock(reactor),
      ui_server(reactor, ui_port)
{
    prepareDiscovery(discover_addr, ctrl_port);
    refreshMenu();
}

void Receiver::prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port) {
    SockAddr discovery_broadcast_addr = make_sockaddr(discover_addr, ctrl_port);

    discovery_sock.setOnReceived([this](SockAddr source, const std::string& message) {
        std::smatch smatch;
        if (std::regex_match(message, smatch, REPLY_REGEX)) {
            try {
                if (smatch.size() != 4) throw std::runtime_error("wrong amount of matches");
                std::string ip_s = smatch[1];
                IpAddr ip = ipaddr_from_string(ip_s);

                int port_i = stoi(smatch[2]);
                if (port_i < 0 || port_i > MAX_PORT_NUMBER) throw std::runtime_error("port out of range");
                auto port = static_cast<uint16_t>(port_i);

                Station station;
                station.name = smatch[3];
                station.addr = make_sockaddr(ip, port);
                dbg << "Discovered " << station.name << " @ " << station.addr << "\n";

                handleReplyFrom(station);
            } catch (std::runtime_error& e) {
                dbg << "Error parsing response " << e.what() << " in " << message << "\n";
            }

        } else {
            dbg << "Unknown message: " << message << "\n";
        }
    });

    reactor.runEvery(DISCOVER_PERIOD_MS, [this, discovery_broadcast_addr]() {
        discovery_sock.send(discovery_broadcast_addr, LOOKUP);
    }, Reactor::RunEveryStartType::START_NOW);
}

void Receiver::handleReplyFrom(Receiver::Station station) {
    auto now = chrono::now();
    chrono::duration dt = std::chrono::milliseconds(DISCOVER_TIMEOUT);
    stations.insert_or_assign(station, now); // set that the station has been heard from recently
    stationListHasChanged();

    reactor.runAt(now + dt, [this, station]() {
        auto it = stations.find(station);
        if (it == stations.end()) return; // station already removed

        if (chrono::to_millis(chrono::now() - it->second) >= DISCOVER_TIMEOUT - 1) { // TODO maybe that -1 is not needed?
            stations.erase(it);
            stationListHasChanged();
        }
    });
}

void Receiver::stationListHasChanged() {
    dbg << "Current stations:\n";
    for (auto s : stations) {
        dbg << s.first.name << " @ " << s.first.addr << "\n";
    }
    refreshMenu();
}

static const char* bar = "------------------------------------------------------------------------";

void Receiver::refreshMenu() {
    std::stringstream ss;
    ss << bar << "\n";
    ss << "  SIK Radio\n";
    ss << bar << "\n";
    for (auto s : stations) {
        // TODO if current - ">"
        if (false) ss << "  > ";
        else ss << "    ";

        ss << s.first.name << "\n";
    }
    ss << bar << "\n";
    ui_server.setWindowContents(ss.str());
}
