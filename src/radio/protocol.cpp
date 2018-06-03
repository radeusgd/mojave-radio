#include "radio/protocol.h"
#include <endian.h>
#include <cstring>

const char* LOOKUP = "ZERO_SEVEN_COME_IN";
const char* REPLY = "BOREWICZ_HERE";
const char* REXMIT = "LOUDER_PLEASE";
std::regex const REXMIT_REGEX(std::string(REXMIT) + " (\\d+(?:,\\d+)*)");

BytesBuffer AudioPackage::pack(const AudioPackage &pkg) {
    BytesBuffer buff;
    buff.resize(2 * sizeof(uint64_t) + pkg.audio_data.size());

    uint64_t tmp;
    tmp = htobe64(pkg.session_id);
    memcpy(&buff[0], &tmp, sizeof(uint64_t));
    tmp = htobe64(pkg.first_byte_num);
    memcpy(&buff[0] + sizeof(uint64_t), &tmp, sizeof(uint64_t));

    memcpy(&buff[0] + 2 * sizeof(uint64_t), &pkg.audio_data[0], pkg.audio_data.size());

    return buff;
}

AudioPackage AudioPackage::unpack(const BytesBuffer &pkg) {
    // TODO assert package size == psize, probably in call-site
    return AudioPackage{}; // TODO implement
}
