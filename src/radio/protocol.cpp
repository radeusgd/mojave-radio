#include "radio/protocol.h"
#include <endian.h>
#include <cstring>

const char* LOOKUP = "ZERO_SEVEN_COME_IN";
const char* REPLY = "BOREWICZ_HERE";
const char* REXMIT = "LOUDER_PLEASE";
std::regex const REXMIT_REGEX(std::string(REXMIT) + " (\\d+(?:,\\d+)*)");
std::regex const REPLY_REGEX(std::string(REPLY) + " ([\\d.]+) (\\d+) (.*)");

BytesBuffer AudioPackage::pack(const AudioPackage &pkg) {
    constexpr size_t S = sizeof(uint64_t);
    BytesBuffer buff;
    buff.resize(2 * S + pkg.audio_data.size());

    uint64_t tmp;
    tmp = htobe64(pkg.session_id);
    memcpy(&buff[0], &tmp, S);
    tmp = htobe64(pkg.first_byte_num);
    memcpy(&buff[0] + S, &tmp, S);

    memcpy(&buff[0] + 2 * S, &pkg.audio_data[0], pkg.audio_data.size());

    return buff;
}

AudioPackage AudioPackage::unpack(const BytesBuffer &buff) {
    constexpr size_t S = sizeof(uint64_t);
    AudioPackage pkg;
    uint64_t tmp;
    memcpy(&tmp, &buff[0], S);
    pkg.session_id = be64toh(tmp);
    memcpy(&tmp, &buff[0] + S, S);
    pkg.first_byte_num = be64toh(tmp);
    pkg.audio_data.resize(buff.size() - 2 * S);
    memcpy(&pkg.audio_data[0], &buff[0] + 2 * S, pkg.audio_data.size());
    return pkg;
}
