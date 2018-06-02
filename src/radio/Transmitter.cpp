//
// Created by radeusgd on 02.06.18.
//

#include "utils/errors.h"
#include "Transmitter.h"
#include <unistd.h>

void Transmitter::sendReply(SockAddr destination) {
    ctrl_sock.send(destination, LOOKUP_REPLY);
}

Transmitter::Transmitter(Reactor &reactor,
                         IpAddr multicast_addr, uint16_t data_port, uint16_t ctrl_port,
                         size_t psize, size_t fsize, int rtime,
                         const std::string &name)
    : reactor(reactor),
      psize(psize),
      ctrl_sock(reactor, make_sockaddr(multicast_addr, ctrl_port), MulticastMode::REGISTER_TO_MULTICAST_GROUP),
      data_sock(reactor, make_sockaddr(multicast_addr, data_port), MulticastMode::MULTICAST_SEND_ONLY)
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

    prepareControl();
    prepareStdin();
    prepareRetransmissions(rtime);
}


void Transmitter::prepareControl() {
    ctrl_sock.setOnReceived([this](SockAddr source, const std::string& message) {
        if (message == LOOKUP) {
            dbg << "Lookup request from " << source << "\n";
            sendReply(source);
        } else if (message == REXMIT) {
            dbg << "Retransmission request from " << source << "\n";
        } else {
            dbg << "Unknown message: " << message << "\n";
        }
    });
}

void Transmitter::prepareStdin() {
    stdin_buff.resize(psize);
    // start reading from stdin
    reactor.setOnReadable(0, [this]() {
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
        if (!rexmit_requests.empty()) {
            // TODO
        }
    });
}

void Transmitter::processPackage(BytesBuffer&& data) {
    assert(data.size() == psize);

    AudioPackage pkg;
    pkg.session_id = session_id;
    pkg.first_byte_num = first_byte_num;
    first_byte_num += psize;
    pkg.audio_data = std::move(data);

    BytesBuffer packed = AudioPackage::pack(pkg);
    // TODO store package for REXMIT

    data_sock.broadcast(packed);
}
