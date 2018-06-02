//
// Created by radeusgd on 02.06.18.
//

#ifndef MOJAVE_RADIO_FIFOBUFFER_H
#define MOJAVE_RADIO_FIFOBUFFER_H


#include <deque>
#include <io/io.h>
#include <cassert>

class FIFOBuffer {
    size_t capacity;
    std::deque<char> buffer; // TODO more efficient implementation ??
public:
    size_t size() const {
        return buffer.size();
    }

    size_t freeSpace() const {
        return capacity - buffer.size();
    }

    void write(const BytesBuffer& data) {
        assert (data.size() <= freeSpace());

        buffer.insert(buffer.end(), data.begin(), data.end());
    }

    BytesBuffer read(size_t max_size) {
        max_size = std::min(max_size, size());
        BytesBuffer tmp(buffer.begin(), buffer.begin() + max_size);
        buffer.erase(buffer.begin(), buffer.begin() + max_size);
    }

    //TODO
};


#endif //MOJAVE_RADIO_FIFOBUFFER_H
