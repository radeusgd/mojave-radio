
#include <boost/program_options.hpp>
#include <iostream>
#include <io/Reactor.h>
#include <utils/logging.h>
#include "utils/constants.h"
#include "net/net.h"
#include "Transmitter.h"

namespace po = boost::program_options;

int main(int argc, const char *argv[]) {
    dbg << "hello\n";
    po::options_description desc("Usage");
    std::string MCAST_ADDR_STR;
    IpAddr MCAST_ADDR;
    uint16_t DATA_PORT;
    uint16_t CTRL_PORT;
    int PSIZE, FSIZE, RTIME;
    std::string NAZWA;
    desc.add_options()
        (",a", po::value<std::string>(&MCAST_ADDR_STR), "MCAST_ADDR")
        (",P", po::value<uint16_t>(&DATA_PORT)->default_value(20000 + NR_ALBUMU_MOD), "DATA_PORT")
        (",C", po::value<uint16_t>(&CTRL_PORT)->default_value(30000 + NR_ALBUMU_MOD), "CTRL_PORT")
        (",p", po::value<int>(&PSIZE)->default_value(512), "PSIZE")
        (",f", po::value<int>(&FSIZE)->default_value(128 * 1024), "FSIZE")
        (",R", po::value<int>(&RTIME)->default_value(250), "RTIME")
        (",n", po::value<std::string>(&NAZWA)->default_value("Nienazwany nadajnik"), "NAZWA");

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        MCAST_ADDR = IpAddr::from_string(MCAST_ADDR_STR);
    } catch (std::exception &e) { // common ancestor for boost::program_options::invalid_option_value and std::runtime_error
        std::cerr << e.what() << "\n" << desc;
        return 1;
    }

    dbg << "MCAST_ADDR = " << MCAST_ADDR << "\n";
    dbg << "NAME = " << NAZWA << "\n";
    dbg << "CTRL_PORT = " << CTRL_PORT << "\n";
    dbg << "DATA_PORT = " << DATA_PORT << "\n";
    dbg << "PSIZE = " << PSIZE << "\n";

    {
        Reactor reactor;
        Transmitter transmitter(reactor,
                                MCAST_ADDR, DATA_PORT, CTRL_PORT,
                                PSIZE, FSIZE, RTIME,
                                NAZWA);
        reactor.run();
    }

    dbg << "Clean exit\n";
    return 0;
}