#include "uci.hpp"
#include "commands.hpp"
#include <iostream>
#include <string>

namespace uci {

void loop() {
    UciState st; // holds Position etc.

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!handle_command(st, line)) break; // false => quit
    }
}

} // namespace uci
