//
// Created by radeusgd on 02.06.18.
//

#include "utils/errors.h"
#include "Transmitter.h"
#include "utils/string.h"
#include "utils/functional.h"
#include <unistd.h>

void Transmitter::sendReply(SockAddr destination) {
    ctrl_sock.send(destination, LOOKUP_REPLY);
}

Transmitter::Transmitter(Reactor &reactor,
                         IpAddr multicast_addr, uint16_t data_port, uint16_t ctrl_port,
                         size_t psize, size_t fsize, int rtime,
                         const std::string &name)
    : reactor(reactor),
      ctrl_sock(reactor, ctrl_port),
      data_sock(reactor),
      broadcast_destination(SockAddr(multicast_addr, data_port)),
      psize(psize),
      fifo(fsize / psize)
{
    // we are not listening on data_port
    data_sock.setOnReceived([](SockAddr source, const BytesBuffer& data) {});

    {
        // prepare reply string
        std::stringstream ss;
        ss << REPLY << " " << multicast_addr << " " << data_port << " " << name;
        LOOKUP_REPLY = ss.str();
    }

    time_t now = time(NULL);
    session_id = static_cast<uint64_t>(now);

    //ctrl_sock.registerToMulticastGroup(multicast_addr); // TODO maybe no need to register to mcast?
    prepareControl();
    prepareStdin();
    prepareRetransmissions(rtime);
}

static uint64_t str_to_uint64(std::string s) {
    std::stringstream ss(s);
    uint64_t x;
    ss >> x;
    return x;
}


void Transmitter::prepareControl() {
    ctrl_sock.setOnReceived([this](SockAddr source, const std::string& message) {
        std::smatch regex_match_result;
        if (message == LOOKUP) {
            dbg << "Lookup request from " << source << "\n";
            sendReply(source);
        } else if (std::regex_match(message, regex_match_result, REXMIT_REGEX)) {
            if (regex_match_result.size() != 2) {
                dbg << "Malformed REXMIT: " << message << "\n";
                return;
            }
            dbg << message << "\n";

            dbg << "Retransmission request from " << source << "\n";
            std::string rexmit_list_s = regex_match_result[1];
            std::vector<uint64_t> rexmit_list = map(split(rexmit_list_s),
                std::function<uint64_t(std::string)>(str_to_uint64));
            for (auto first_byte_num : rexmit_list) {
                if (first_byte_num % psize != 0) {
                    dbg << first_byte_num << " is not divisible by psize (" << psize << ")\n";
                    continue;
                }
                rexmit_requests.insert(first_byte_num / psize);
            }
        } else {
            dbg << "Unknown message: " << message << "\n";
        }
    });
}

void Transmitter::prepareStdin() {
    stdin_buff.resize(psize);
    static constexpr int STDIN = 0;
    reactor.setOnReadable(STDIN, [this]() {
        assert(in_buffer < psize);
        size_t to_read = psize - in_buffer;
        //dbg << "Want to read " << to_read << std::endl;
        ssize_t r = read(0, &stdin_buff[0] + in_buffer, to_read);
        //dbg << "Did read " << r << std::endl;

        if (r < 0) {
            raise_errno("stdin read");
        }

        if (r == 0) {
            // we reached end of file - stop
            dbg << "stdin - EOF\n";
            reactor.cancelReading(0);
            reactor.stop(); // TODO right now?
            return;
        }

        in_buffer += r;

        if (in_buffer == psize) {
            processPackage(std::move(stdin_buff)); // move contents
            stdin_buff.resize(psize); // bring buffer back to useful state
            in_buffer = 0;
        }
    });
}

void Transmitter::prepareRetransmissions(int rtime) {
    reactor.runEvery(rtime, [this]() {
        for (auto requested_pkg_id : rexmit_requests) {
            if (!fifo.has(requested_pkg_id)) {
                dbg << "Cannot retransmit " << requested_pkg_id << "\n";
                continue; // drop requests for unavailable data
            }

            dbg << "Retransmitting " << requested_pkg_id << "\n";
            data_sock.send(broadcast_destination, fifo.get(requested_pkg_id));
        }
        rexmit_requests.clear();
    });
}

void Transmitter::processPackage(BytesBuffer&& data) {
    assert(data.size() == psize);

    AudioPackage pkg;
    pkg.session_id = session_id;
    pkg.first_byte_num = first_byte_num;
    assert(pkg.first_byte_num % psize == 0);
    first_byte_num += psize;
    pkg.audio_data = std::move(data);

    BytesBuffer packed = AudioPackage::pack(pkg);
    fifo.append(packed);

    data_sock.send(broadcast_destination, packed);
}
