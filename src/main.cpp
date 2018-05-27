#include <iostream>
#include "io/Reactor.h"

int main() {
    Reactor reactor;

    reactor.runEvery(500, [&reactor](){

    });
    reactor.run();
    return 0;
}