#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "../../chess/move.hpp"
#include "../../chess/position.hpp"
#include "../../eval/eval.hpp" // for eval::Evaluator delta

namespace search::util {

struct ScoredMove {
    chess::Move m = 0;
    int score = 0;
};

inline void sort_moves(std::vector<ScoredMove>& v) {
    std::sort(v.begin(), v.end(),
              [](const ScoredMove& a, const ScoredMove& b) { return a.score > b.score; });
}

// small helpers
inline bool is_capture_like(chess::Move m) {
    const uint32_t fl = chess::flags(m);
    return (fl & chess::CAPTURE_MOVE) || (fl & chess::EP) || (fl & chess::PROMO);
}

// NOTE: this is *pure* ordering sugar: it does not need SEE yet.
// It uses eval delta if available + strong bias for captures/promos.
inline int score_move(const chess::Position& pos, eval::Evaluator& ev, chess::Move m) {
    int s = 0;
    const uint32_t fl = chess::flags(m);

    // big buckets first
    if (fl & chess::PROMO)        s += 500000;
    if (fl & chess::EP)           s += 400000;
    if (fl & chess::CAPTURE_MOVE) s += 300000;

    // eval delta seasoning
    auto d = ev.estimate_delta(pos, m);
    if (d.valid) {
        s += 1000 * d.cp; // scale so +0.2 pawn matters a bit but doesn't dominate buckets
        if (d.affects_restriction) s += 2500;
    }

    // deterministic tie-breaker to keep ordering stable
    s += (int)chess::to(m) - (int)chess::from(m);

    return s;
}

} // namespace search::util