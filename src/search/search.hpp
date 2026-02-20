#pragma once

#include <cstdint>
#include "../eval/eval.hpp"
#include "../chess/position.hpp"
#include "../chess/move.hpp"

namespace search {

struct Limits {
    int depth = 8;          // max depth
    int movetime_ms = 0;    // 0 => ignore (you can add time later)
};

struct Result {
    chess::Move best = chess::NO_MOVE;
    int score = 0;               // centipawns from root side
    int depth = 0;               // depth completed
    std::uint64_t nodes = 0;     // total nodes searched
    int elapsed_ms = 0;
};

Result think(chess::Position& pos, const Limits& lim, int movetime_ms = 0);
} // namespace search