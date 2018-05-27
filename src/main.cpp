#include <iostream>
#include "io/Reactor.h"
#include "io/StdinReader.h"

int main() {
    Reactor reactor;

    reactor.runEvery(500, [&reactor](){
        std::cout<<"Hi\n";
    });

    read_from_stdin(reactor, [](BytesBuffer buff) {
        buff.push_back('\0');
        std::cout << "Read: '" << &buff[0] << "'\n";
    },
    [&reactor]() {
        std::cout << "EOF\n";
        reactor.stop();
    });


    reactor.run();

    return 0;
}