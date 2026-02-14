#pragma once

#include <cstdint>
#include "position.hpp"

namespace chess {

struct Zobrist {
    std::uint64_t piece[2][6][64]{};
    std::uint64_t castling[16]{};
    std::uint64_t ep_file[9]{};
    std::uint64_t side{};

    void init(std::uint64_t seed = 0x9e3779b97f4a7c15ULL);
};

extern Zobrist ZB;

std::uint64_t compute_key(const Position& pos);

} // namespace chess
