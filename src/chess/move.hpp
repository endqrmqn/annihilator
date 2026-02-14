#pragma once

#include "types.hpp"

namespace chess {
using Move = uint32_t;
constexpr Move NO_MOVE = 0;

enum MoveFlag : uint32_t {
    QUIET_MOVE      = 0,
    CAPTURE_MOVE    = 1 << 0,
    EP              = 1 << 1,
    CASTLE          = 1 << 2,
    DPUSH           = 1 << 3,
    PROMO           = 1 << 4
};

inline Move make_move(Square from, Square to, uint32_t flags = 0, uint32_t promo = 0) {
    return (from) | (to << 6) | (promo << 12) | (flags << 16);
}

inline Square from(Move m){return m & 0x3F;}
inline Square to(Move m){return (m >> 6) & 0x3F;}
inline uint32_t promo(Move m){return (m >> 12) & 0xF;}
inline uint32_t flags(Move m){return (m >> 16);}
}