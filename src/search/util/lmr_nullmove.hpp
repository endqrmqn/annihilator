#pragma once

#include "../../chess/move.hpp"

namespace search::util {

// conservative default reductions; tune later
inline int lmr_reduction(int depth, int move_index, bool is_capture) {
    if (depth < 3) return 0;
    if (move_index < 3) return 0;
    if (is_capture) return 0;

    // very mild: reduce more for later quiet moves
    int r = 1;
    if (depth >= 6 && move_index >= 8) r = 2;
    return r;
}

} // namespace search::util