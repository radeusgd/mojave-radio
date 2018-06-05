//
// Created by radeusgd on 05.06.18.
//

#include "StdoutWriter.h"
#include <unistd.h>
#include <utils/errors.h>

StdoutWriter::StdoutWriter(Reactor &reactor)
    : reactor(reactor) {

}

void StdoutWriter::write(const BytesBuffer &data, std::function<void()> finished) {
    if (!buff.empty()) {
        throw std::runtime_error("Already writing!");
    }
    buff = data;
    reactor.setOnWriteable(STDOUT, [this, finished]() {
        assert (written < buff.size());
        size_t to_write = buff.size() - written;
        ssize_t r = ::write(STDOUT, &buff[written], to_write);
        if (r < 0) {
            raise_errno("stdout write");
        }

        written += r;
        if (written == buff.size()) {
            buff.clear();
            written = 0;
            reactor.cancelWriting(STDOUT);
            finished();
        }
    });
}
