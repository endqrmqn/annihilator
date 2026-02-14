#pragma once

#include <string>
#include "../chess/position.hpp"

namespace uci {

struct UciState {
    chess::Position pos;
};

bool handle_command(UciState& st, const std::string& line);

} // namespace uci
