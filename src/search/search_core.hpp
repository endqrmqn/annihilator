#pragma once

#include <cstdint>
#include <chrono>

#include "../chess/position.hpp"
#include "../eval/eval.hpp"

#include "util/tt.hpp"
#include "../chess/zobrist.hpp" // for chess::compute_key

namespace search {

struct State {
    eval::Evaluator eval;

    std::uint64_t nodes = 0;

    TranspositionTable tt;

    // timing
    std::chrono::steady_clock::time_point start;
    int time_limit_ms = 0;   // 0 = ignore
    bool stopped = false;

    inline int elapsed_ms() const {
        using namespace std::chrono;
        return (int)duration_cast<milliseconds>(
            steady_clock::now() - start
        ).count();
    }

    inline bool time_up() {
        if (time_limit_ms <= 0) return false;
        if (elapsed_ms() >= time_limit_ms) {
            stopped = true;
            return true;
        }
        return false;
    }
};

int negamax(State& st, chess::Position& pos, int depth, int alpha, int beta, int ply);
} // namespace search