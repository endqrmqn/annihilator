#pragma once

#include "position.hpp"
#include "attacks.hpp"

namespace chess {

bool is_square_attacked(const Position& pos, Square sq, Color by);

inline bool in_check(const Position& pos, Color c) {
    Square ksq = pos.king_square(c);
    if (ksq == NO_SQUARE) return false;
    return is_square_attacked(pos, ksq, ~c);
}

} // namespace chess
