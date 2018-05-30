#include <iostream>
#include "net/TextMulticastSocket.h"
#include "io/Reactor.h"
#include "io/StdinReader.h"
#include "net/net.h"

class EchoSocket : public TextMulticastSocket {
public:
    EchoSocket(Reactor &reactor, const IpAddr &multicast_address)
        : TextMulticastSocket(reactor, multicast_address) {}
    void onReceived(IpAddr sender, const std::string& message) override {
        std::cout << message << "\n"; // TODO IP
        // TODO use stdoutwriter
    }
};

int main() {
    Reactor reactor;
    std::string ipstr = "239.10.11.12:40000";

    EchoSocket socket(reactor, ipaddr_from_string(ipstr));

    reactor.runEvery(5000, [&reactor](){
        std::cout<<"Hi\n";
    });

    read_from_stdin(reactor, [&socket](BytesBuffer buff) {
        buff.push_back('\0');
        std::string str = &buff[0];
        std::cout << "Read: '" << str << "'\n";
        socket.broadcast(str);
    },
    [&reactor]() {
        std::cout << "EOF\n";
        reactor.stop();
    });


    reactor.run();

    return 0;
}