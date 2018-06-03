//
// Created by radeusgd on 02.06.18.
//

#ifndef MOJAVE_RADIO_PROTOCOL_H
#define MOJAVE_RADIO_PROTOCOL_H

#include <cstdint>
#include <io/io.h>
#include <regex>

extern const char* LOOKUP;
extern const char* REPLY;
extern const char* REXMIT;
extern const std::regex REXMIT_REGEX;

struct AudioPackage {
    uint64_t session_id;
    uint64_t first_byte_num;
    BytesBuffer audio_data;

    static BytesBuffer pack(const AudioPackage &pkg);
    static AudioPackage unpack(const BytesBuffer &pkg);
};

#endif //MOJAVE_RADIO_PROTOCOL_H
