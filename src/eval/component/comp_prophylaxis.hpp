#pragma once

#include "../eval_component.hpp"
#include "../../chess/position.hpp"
#include "../../chess/move.hpp"
#include "../../chess/movegen.hpp"
#include "../../chess/make.hpp"
#include "../../chess/legality.hpp"

#include <vector>

namespace eval {

// “suffocation” / restriction: primarily opponent legal move count.
// safe fallback: recompute counts on every make/unmake (fast enough for now; optimize later).
struct CompProphylaxis {
    void init(const chess::Position& pos) { recompute(pos); }

    PhaseScore value(const chess::Position& pos, chess::Color us) const;

    void on_make_move(const chess::Position& pos, chess::Move) { recompute(pos); }
    void on_unmake_move(const chess::Position& pos, chess::Move) { recompute(pos); }

    MoveDelta estimate_delta(const chess::Position& pos, chess::Move m) const;

private:
    // cached legal move counts by side in the *current* position
    int legal_ct[2]{0,0};

    void recompute(const chess::Position& pos);

    static int legal_moves_for_side(const chess::Position& pos, chess::Color side);
};

} // namespace eval