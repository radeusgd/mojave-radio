//
// Created by radeusgd on 05.06.18.
//

#ifndef MOJAVE_RADIO_STDOUTWRITER_H
#define MOJAVE_RADIO_STDOUTWRITER_H

#include "Reactor.h"
#include "io.h"

/*
 * Writes stdout as fast as possible but with cooperation of data source
 * (so that data isn't taken from source faster than really written).
 */
class StdoutWriter {
private:
    static constexpr int STDOUT = 1;
    Reactor& reactor;
    BytesBuffer buff;
    size_t written = 0;
public:
    explicit StdoutWriter(Reactor& reactor);
    void write(const BytesBuffer& data, std::function<void()> finished);

    bool is_writing() const {
        return !buff.empty();
    }

    void cancel() {
        reactor.cancelWriting(STDOUT);
        buff.clear();
        written = 0;
    }

    ~StdoutWriter();
};


#endif //MOJAVE_RADIO_STDOUTWRITER_H
