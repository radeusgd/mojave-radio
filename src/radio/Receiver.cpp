//
// Created by radeusgd on 04.06.18.
//

#include "utils/string.h"
#include "utils/functional.h"
#include "Receiver.h"
#include "protocol.h"
#include <utility>

Receiver::Receiver(Reactor& reactor,
                   IpAddr discover_addr, uint16_t ctrl_port, uint16_t ui_port,
                   size_t bsize, int rtime,
                   std::string default_station_name)
    : reactor(reactor),
      discovery_sock(reactor),
      audio_sock(reactor),
      stdout_writer(reactor),
      ui_server(reactor, ui_port),
      default_station_name(std::move(default_station_name)),
      bsize(bsize),
      rexmit_time(rtime),
      buffer(bsize)
{
    prepareDiscovery(discover_addr, ctrl_port);
    prepareMenu();
    prepareAudio();
}

void Receiver::prepareDiscovery(IpAddr discover_addr, uint16_t ctrl_port) {
    SockAddr discovery_broadcast_addr = SockAddr(discover_addr, ctrl_port);

    discovery_sock.setOnReceived([this](SockAddr source, const std::string& message) {
        std::smatch smatch;
        if (std::regex_match(message, smatch, REPLY_REGEX)) {
            try {
                if (smatch.size() != 4) throw std::runtime_error("wrong amount of matches");
                std::string ip_s = smatch[1];
                IpAddr ip = IpAddr::from_string(ip_s);

                int port_i = stoi(smatch[2]);
                if (port_i < 0 || port_i > MAX_PORT_NUMBER) throw std::runtime_error("port out of range");
                auto port = static_cast<uint16_t>(port_i);

                handleReplyFrom(Station{
                        .name = smatch[3],
                        .audio_mcast_addr = SockAddr(ip, port),
                        .rexmit_addr = source
                });
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
        if (it == stations.end()) {
            dbg << "\n\n\nTHIS SHOULDN'T HAPPEN!!!\n";
            dbg << __FILE__ << ":" << __LINE__ << "\n";
            dbg << current_station->name << ", " << current_station->audio_mcast_addr << "\n";
            dbg << "Recovering...\n";
            stationListHasChanged();
        }

        auto new_station = mod(stations, it)->first;
        if (new_station != *current_station) {
            changeCurrentStation(new_station);
        }
    }
}

void Receiver::changeCurrentStation(Station new_station) {
    if (current_station) {
        stopListening(*current_station);
    }
    current_station = new_station;
    startListening(new_station);
    refreshMenu();
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
    });
    refreshMenu();

#ifdef DEBUG_OVER_TELNET
    reactor.runEvery(100, [this]() { refreshMenu(); });
#endif
}

void Receiver::handleReplyFrom(Receiver::Station station) {
    dbg << "Discovered " << station.name << " @ " << station.audio_mcast_addr << "\n";
    auto now = chrono::now();
    stations.insert_or_assign(station, now); // set that the station has been heard from recently
    stationListHasChanged();

    reactor.runAfter(DISCOVER_TIMEOUT, [this, station]() {
        auto it = stations.find(station);
        if (it == stations.end()) return; // station already removed

        // I'm checking for DISCOVER_TIMEOUT - 2, because of rounding issues (runAfter implementation has +-1ms precision)
        // this doesn't change the time of when the station goes out, because runAfter is set to exactly 20s anyway
        if (chrono::to_millis(chrono::now() - it->second) >= DISCOVER_TIMEOUT - 2) {
            dbg << "Station " << it->first.name << " signal lost.\n";
            stations.erase(it);
            stationListHasChanged();
        }
    });
}

void Receiver::stationListHasChanged() {
    /*
     * In response to https://szkopul.edu.pl/c/sik2017l/questions/2521/
     * I decided that if the default station is specified it is selected whenever it becomes available.
     * This makes the menu unusable when this station is present, but if someone wanted a default station, that makes sense to me.
     */
    if (!default_station_name.empty()) { // if default station name is set
        if (current_station
            && current_station->name == default_station_name
            && stations.find(*current_station) != stations.end()) {
            return refreshMenu(); // we are already listening to default and it is alive
        }


        // otherwise check if default is available and start listening
        for (const auto& s : stations) {
            if (s.first.name == default_station_name) {
                dbg << "Switching to default station.\n";
                changeCurrentStation(s.first);
                return;
            }
        }
    }

    if (current_station) {
        // if we are already playing something, check if it's still alive
        if (stations.find(*current_station) == stations.end()) {
            // if lost our current station
            stopListening(*current_station);
            current_station = std::nullopt;
            // rerun update to try to listen to some other station if available
            return stationListHasChanged();
        }
    } else {
        // we are listening to nothing
        // if there's any station available, pick the first one
        if (!stations.empty()) {
            current_station = stations.begin()->first;
            startListening(*current_station);
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
    for (const auto &s : stations) {
        if (current_station && *current_station == s.first) {
            ss << "  > ";
        } else {
            ss << "    ";
        }

        ss << s.first.name << "\n";
    }
    ss << bar << "\n";

#ifdef DEBUG_OVER_TELNET
    ss << "\n\n" << buffer << "\n";
#endif

    ui_server.setWindowContents(ss.str());
}

void Receiver::startListening(Receiver::Station station) {
    resetSession();
    dbg << "Starting listening to " << station.name << " @ " << station.audio_mcast_addr << "\n";
    audio_sock.rebind(station.audio_mcast_addr.port());
    audio_sock.registerToMulticastGroup(station.audio_mcast_addr.ip());
}

void Receiver::stopListening(Receiver::Station station) {
    // don't know if do anything actually
    dbg << "Stopping listening to " << station.name << " @ " << station.audio_mcast_addr << "\n";
    resetSession();
}

void Receiver::prepareAudio() {
    audio_sock.setOnReceived([this](SockAddr source, const BytesBuffer& data) {
        AudioPackage pkg = AudioPackage::unpack(data);
        handleIncomingPackage(std::move(pkg));
    });
}

void Receiver::handleIncomingPackage(AudioPackage &&pkg) {
    if (!current_station) return; // if not listening to anything, something strange happened so drop the package

    if (session) {
        // there's a session ongoing, verify correctness
        if (pkg.session_id != session->id) {
            return; // drop packets from different session
        }
        if (pkg.audio_data.size() != session->psize) {
            dbg << "A packet has inconsistent PSIZE in the same session!\n";
            dbg << "Session PSIZE = " << session->psize
                << ", packet PSIZE = " << pkg.audio_data.size() << "\n";
            return;
        }
    } else {
        // start a new session
        uint64_t psize = pkg.audio_data.size();
        uint64_t pkg_id = pkg.first_byte_num / psize;
        session = Session{
            .id = pkg.session_id,
            .psize = psize,
            .byte0 = pkg.first_byte_num,
            .byte_to_start_bursting = pkg.first_byte_num + 3 * bsize / 4,
            .latest_received_pkg_id = pkg_id,
            .bursting_pkg_id = std::nullopt
        };
    }

    uint64_t pkg_id = pkg.first_byte_num / session->psize;

    buffer.insert(pkg_id, std::move(pkg.audio_data));
    if (!buffer.has(pkg_id)) {
        dbg << "Packet " << pkg_id << " hasn't been added. Oldest packet in buffer = " << buffer.firstPacketId() << "\n";
    }

    if (pkg_id > session->latest_received_pkg_id) {
        // new biggest pkg id, handle new REXMITs
        std::set<uint64_t> rexmit_ids; // will rexmit byte numbers
        for (auto i = session->latest_received_pkg_id + 1; i < pkg_id; ++i) {
            if (buffer.canHave(i) && !buffer.has(i)) {
                rexmit_ids.insert(i);
            }
        }
        session->latest_received_pkg_id = pkg_id;

        if (!rexmit_ids.empty()) {
            scheduleRexmitRequest(rexmit_ids, session->id);
        }
    }

    if (!session->bursting_pkg_id && pkg.first_byte_num >= session->byte_to_start_bursting) {
        // if current byte is BYTE0 + floor(BSIZE*3/4) and we're not yet playing, start playing
        session->bursting_pkg_id = buffer.firstPacketId();
        writeToStdout();
    }
}

void Receiver::scheduleRexmitRequest(std::set<uint64_t> missing_ids, uint64_t session_id) {
    reactor.runAfter(rexmit_time, [this, missing_ids, session_id]() {
        if (!session || session_id != session->id) return; // if we changed the session all rexmits are to be cancelled

        std::set<uint64_t> new_ids;
        for (auto x : missing_ids) {
            // check if the packet is still wanted
            if (buffer.canHave(x) && !buffer.has(x)) {
                new_ids.insert(x);
            }
        }

        if (new_ids.empty()) return; // if nothing is left to rexmit, return

        dbg << "Asking for REXMIT of " << new_ids.size() << " / " << missing_ids.size() << " packets\n";
        sendRexmitRequest(new_ids); // send the request
        scheduleRexmitRequest(new_ids, session_id); // schedule another try
    });
}

void Receiver::sendRexmitRequest(std::set<uint64_t> missing_ids) {
    assert(!missing_ids.empty());
    assert(session);
    assert(current_station);

    std::stringstream ss;
    ss << REXMIT << " ";
    bool first = true;
    for (auto id : missing_ids) {
        if (first) {
            first = false;
        } else {
            ss << ",";
        }

        uint64_t bytenum = id * session->psize;
        ss << bytenum;
    }

    //dbg << "Sending " << ss.str() << " to " << current_station->rexmit_addr << "\n";
    discovery_sock.send(current_station->rexmit_addr, ss.str());
}

void Receiver::writeToStdout() {
    if (!session || !session->bursting_pkg_id) {
        dbg << "Session invalidated during writing! Shouldn't happen.\n";
        return;
    }

    if (stdout_writer.is_writing()) {
        // if something was still being written, cancel that
        // this is unlikely to happen, but may happen when switching channels
        stdout_writer.cancel();
    }

    auto current_pkg = *session->bursting_pkg_id;

    if (!buffer.has(current_pkg)) {
        dbg << "Packet " << current_pkg << " is missing! Restarting listening...\n";
        resetSession(); // ditch current buffer and session
        return;
    }

    stdout_writer.write(buffer.get(current_pkg), [this]() {
        session->bursting_pkg_id = *session->bursting_pkg_id + 1; // switch to next packet
        writeToStdout();
    });
}

void Receiver::resetSession() {
    session = std::nullopt;
    buffer.reset();
    stdout_writer.cancel();
}
