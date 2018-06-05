//
// Created by radeusgd on 04.06.18.
//

#include <utils/string.h>
#include <utility>
#include "Receiver.h"
#include "protocol.h"

Receiver::Receiver(Reactor& reactor,
                   IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
                   size_t bsize, int rtime,
                   std::string default_station_name)
    : reactor(reactor),
      discovery_sock(reactor),
      ui_server(reactor, ui_port),
      default_station_name(std::move(default_station_name))
{
    prepareDiscovery(discover_addr, ctrl_port);
    prepareMenu();
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

void Receiver::moveMenuChoice(std::function<Stations::iterator(const Stations&, Stations::iterator)> mod) {
    if (current_station) {
        // if no station is playing there's no "choice" to move
        auto it = stations.find(*current_station);
        assert (it != stations.end());
        current_station = mod(stations, it)->first;
    }
}

void Receiver::prepareMenu() {
    ui_server.setInputHandler([this](TelnetServer::InputKey k) {
       switch (k) {
           case TelnetServer::KEY_UP:
               moveMenuChoice([](const Stations& s, Stations::iterator it) {
                    if (it == s.begin()) return it;
                    else return std::prev(it);
               });
               break;
           case TelnetServer::KEY_DOWN:
               moveMenuChoice([](const Stations& s, Stations::iterator it) {
                   auto nn = std::next(it);
                   if (nn == s.end()) return it;
                   else return nn;
               });
               break;
       }
       refreshMenu();
    });
    refreshMenu();
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
            dbg << "Station " << it->first.name << " signal lost.\n";
            stations.erase(it);
            stationListHasChanged();
        }
    });
}

void Receiver::stationListHasChanged() {
    // TODO default_station semantics - what are they?
    if (current_station) {
        // if we are already playing something, check if it's still alive
        if (stations.find(*current_station) == stations.end()) {
            // if lost our current station
            current_station = std::nullopt; // stop listening TODO stop listening
            // rerun update to try to listen to some other station if available
            return stationListHasChanged();
        }
    } else {
        // we are listening to nothing
        // if there's any station available, pick the first one
        if (!stations.empty()) {
            current_station = stations.begin()->first;
            // TODO start listening
        }
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
        if (current_station && *current_station == s.first) {
            ss << "  > ";
        } else {
            ss << "    ";
        }

        ss << s.first.name << "\n";
    }
    ss << bar << "\n";
    ui_server.setWindowContents(ss.str());
}
