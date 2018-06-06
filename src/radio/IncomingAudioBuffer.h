//
// Created by radeusgd on 06.06.18.
//

#ifndef MOJAVE_RADIO_HOLLOWBUFFER_H
#define MOJAVE_RADIO_HOLLOWBUFFER_H


#include <cstddef>
#include <optional>
#include <cassert>
#include <deque>
#include "io/io.h"

class IncomingAudioBuffer {
public:
    using PacketId = size_t;
private:
    size_t max_size;
    struct State {
        size_t psize;
        PacketId first_id; // id of first package that is in buffer
    };
    std::optional<State> state = std::nullopt;
    std::deque<BytesBuffer> data;

    inline size_t capacity() const {
        assert (state);
        return max_size / state->psize;
    }

    inline size_t index(PacketId id) const {
        assert (id >= state->first_id);
        return id - state->first_id;
    }

    void pop_front() {
        data.pop_front();
        state->first_id++;
    }

    void allocateUpTo(PacketId id) {
        while (index(id) >= data.size()) { // while this index is out of range
            data.emplace_back(); // extend the buffer
            if (data.size() > capacity()) {
                pop_front(); // but maintain the maximum capacity
            }
        }
    }

public:
    explicit IncomingAudioBuffer(size_t max_size) : max_size(max_size) {}

    /*
     * Checks if given packet is in the range of what the buffer can contain.
     */
    bool canHave(PacketId id) const {
        if (!state) return false; // undefined buffer
        if (id < state->first_id) return false; // too old
        return index(id) < capacity(); // within capacity
    }

    /*
     * Checks if given packet is present in the buffer.
     */
    bool has(PacketId id) const {
        if (!canHave(id)) return false; // if out of range, discard
        size_t ind = index(id);
        if (ind >= data.size()) return false; // we could but dont' have this frame allocated
        return !data[ind].empty(); // empty BytesBuffer means we don't have that packet
    }

    void insert(PacketId id, BytesBuffer&& packet) {
        if (id < state->first_id) return; // discard too old
        if (!state) {
            state = State{
                .psize = packet.size(),
                .first_id = id,
            };
        }

        allocateUpTo(id);
        data[index(id)] = std::move(packet);
    }

    void reset() {
        state = std::nullopt;
        data.clear();
    };
};


#endif //MOJAVE_RADIO_HOLLOWBUFFER_H
