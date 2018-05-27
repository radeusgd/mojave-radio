//
// Created by radeusgd on 28.05.18.
//

#ifndef MOJAVE_RADIO_STDINREADER_H
#define MOJAVE_RADIO_STDINREADER_H


#include "Reactor.h"
#include "io.h"

void read_from_stdin(Reactor& reactor,
                     std::function<void(BytesBuffer)> on_read,
                     std::function<void()> on_close);


#endif //MOJAVE_RADIO_STDINREADER_H
