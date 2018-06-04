//
// Created by radeusgd on 04.06.18.
//

#include <sstream>
#include "utils/string.h"

std::vector<std::string> split(std::string str, char sep) {
    std::vector<std::string> res;
    std::stringstream ss(str);
    while (!ss.eof()) {
        std::string tmp;
        std::getline(ss, tmp, sep);
        res.push_back(tmp);
    }
    return res;
}
