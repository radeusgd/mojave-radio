//
// Created by radeusgd on 28.05.18.
//

#include "StdinReader.h"
#include <unistd.h>

static constexpr size_t BUFSIZE = 1024;

void read_from_stdin(Reactor &reactor,
                     std::function<void(BytesBuffer)> on_read,
                     std::function<void()> on_close) {
    reactor.setOnReadable(0, [&reactor, on_read, on_close]() {

        std::vector<char> buffer;
        buffer.resize(BUFSIZE);
        ssize_t r = read(0, &buffer[0], BUFSIZE);
        if (r < 0) {
            // TODO error
            return;
        }

        if (r == 0) {
            reactor.cancelReading(0);
            on_close();
        } else {
            buffer.resize(r);
            on_read(buffer);
        }
    });
}
