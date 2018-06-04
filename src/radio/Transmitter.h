//
// Created by radeusgd on 02.06.18.
//

#ifndef MOJAVE_RADIO_NADAJNIK_H
#define MOJAVE_RADIO_NADAJNIK_H


#include <io/Reactor.h>
#include <net/UDPSocket.h>
#include <net/TextUDPSocket.h>
#include <utils/logging.h>
#include <sstream>
#include <io/StdinReader.h>
#include <utils/FIFOBuffer.h>
#include "protocol.h"

class Transmitter {
private:
    Reactor& reactor;
    TextUDPSocket ctrl_sock;
    UDPSocket data_sock;
    SockAddr broadcast_destination;

    std::string LOOKUP_REPLY;
    // initializers
    void prepareControl();
    void prepareStdin();
    void prepareRetransmissions(int rtime);

    void sendReply(SockAddr destination);

    BytesBuffer stdin_buff;
    size_t in_buffer = 0;

    size_t psize;
    uint64_t first_byte_num = 0;
    uint64_t session_id;
    FIFOBuffer<BytesBuffer> fifo;
    std::set<size_t > rexmit_requests;

    void processPackage(BytesBuffer&& data);

public:
    Transmitter(Reactor& reactor,
             IpAddr multicast_addr, uint16_t data_port, uint16_t ctrl_port,
             size_t psize, size_t fsize, int rtime,
             const std::string &name);
};


#endif //MOJAVE_RADIO_NADAJNIK_H
