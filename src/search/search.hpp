#pragma once
#include <cstdint>
#include "../chess/position.hpp"
#include "../chess/move.hpp"

namespace search {

struct Limits {
    int depth = 6;
};

struct Result {
    chess::Move best = chess::NO_MOVE;
    int score = 0;
    std::uint64_t nodes = 0;
};

Result think(chess::Position& pos, const Limits& lim);

} // namespace search
