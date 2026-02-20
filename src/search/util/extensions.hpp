#pragma once

#include "../../chess/move.hpp"

namespace search::util {

// extension in plies
inline int extension_for(chess::Move m) {
    const uint32_t fl = chess::flags(m);

    // easy starting rule set:
    // - promotions extend
    // - captures extend (lightly)
    // later: check extension, passed pawn push, etc.
    if (fl & chess::PROMO) return 1;
    if (fl & (chess::CAPTURE_MOVE | chess::EP)) return 1;
    return 0;
}

} // namespace search::util