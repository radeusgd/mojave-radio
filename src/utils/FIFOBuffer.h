//
// Created by radeusgd on 02.06.18.
//

#ifndef MOJAVE_RADIO_FIFOBUFFER_H
#define MOJAVE_RADIO_FIFOBUFFER_H

#include "io/io.h"
#include <deque>
#include <cassert>
#include <stdexcept>

/*
 * A buffer containing last N added elements and allowing to access any of them.
 * Accessing an element is according to it's index among all elements ever added.
 */
template<typename Elem> class FIFOBuffer {
    size_t capacity;
    std::deque<Elem> buffer;
    ssize_t offset = 0;
    ssize_t posInBuffer(size_t original) const {
        return original - offset;
    }
public:
    explicit FIFOBuffer(size_t capacity) : capacity(capacity) {}

    /*
     * Adds a new element and if the buffer is full, removes an old one.
     */
    void append(const Elem& e) {
        if (buffer.size() == capacity) {
            // if buffer is full, we need to make place for new element
            buffer.pop_front();
            offset++;
        }

        buffer.push_back(e);
        assert(buffer.size() <= capacity);
    }

    /*
     * Returns whether the ith added element is present in the buffer.
     */
    bool has(size_t i) const {
        auto p = posInBuffer(i);
        return 0 <= p && p < buffer.size();
    }

    /*
     * Returns ith added element or throws an exception.
     */
    const Elem& get(size_t i) const {
        if (!has(i)) throw std::out_of_range("Element not available.");
        return buffer[posInBuffer(i)];
    }

};


#endif //MOJAVE_RADIO_FIFOBUFFER_H
