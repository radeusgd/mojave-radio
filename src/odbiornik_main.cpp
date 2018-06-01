#include <boost/program_options.hpp>
#include <iostream>
#include "constants.h"
#include "net/net.h"

namespace po = boost::program_options;

int main(int argc, const char *argv[]) {
    po::options_description desc("Usage");
    std::string DISCOVER_ADDR_STR;
    IpAddr DISCOVER_ADDR;
    uint16_t CTRL_PORT;
    uint16_t UI_PORT;
    int BSIZE, RTIME;
    desc.add_options()
        (",d", po::value<std::string>(&DISCOVER_ADDR_STR)->default_value("255.255.255.255"), "DISCOVER_ADDR")
        (",C", po::value<uint16_t>(&CTRL_PORT)->default_value(30000 + NR_ALBUMU_MOD), "CTRL_PORT")
        (",U", po::value<uint16_t>(&UI_PORT)->default_value(10000 + NR_ALBUMU_MOD), "UI_PORT")
        (",f", po::value<int>(&BSIZE)->default_value(64 * 1024), "BSIZE")
        (",R", po::value<int>(&RTIME)->default_value(250), "RTIME");

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        DISCOVER_ADDR = ipaddr_from_string(DISCOVER_ADDR_STR);
    } catch (std::exception &e) { // common ancestor for boost::program_options::invalid_option_value and std::runtime_error
        std::cerr << e.what() << "\n" << desc;
        return 1;
    }
}