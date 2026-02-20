// src/eval/eval.hpp
#pragma once

#include "../chess/position.hpp"
#include "../chess/move.hpp"
#include "../chess/bitboard.hpp"

#include "eval_component.hpp"
#include "eval_aggregator.hpp"

// components
#include "component/comp_material.hpp"
#include "component/comp_pst.hpp"
#include "component/comp_space.hpp"
// #include "component/comp_prophylaxis.hpp" // keep off for now (too slow)

namespace eval {

using EngineEval = Aggregator<
    CompMaterial,
    CompPST,
    CompSpace
    // ,CompProphylaxis
>;

struct DeltaResult {
    int cp = 0;
    bool valid = false;
    bool affects_restriction = false;
};

class Evaluator {
public:
    void init(const chess::Position& pos) { agg_.init(pos); }

    void on_make_move(const chess::Position& pos, chess::Move m) { agg_.on_make_move(pos, m); }
    void on_unmake_move(const chess::Position& pos, chess::Move m) { agg_.on_unmake_move(pos, m); }

    int eval_stm_cp(const chess::Position& pos) const {
        PhaseScore ps = agg_.value(pos, pos.stm);
        return blend(pos, ps);
    }

    DeltaResult estimate_delta(const chess::Position& pos, chess::Move m) const {
        MoveDelta d = agg_.estimate_delta(pos, m);
        if (!d.valid) return {};
        return { blend(pos, d.delta), true, d.affects_restriction };
    }

private:
    EngineEval agg_{};

    static int phase256(const chess::Position& pos) {
        // KN=1, BI=1, RO=2, QU=4; max=24
        constexpr int wN=1, wB=1, wR=2, wQ=4, maxPhase=24;

        auto cnt = [&](chess::Color c, chess::PieceType pt) -> int {
            return chess::popcount(pos.pieces[(int)c][(int)pt]);
        };

        int ph = 0;
        for (int c = 0; c < 2; ++c) {
            ph += wN * cnt((chess::Color)c, chess::KNIGHT);
            ph += wB * cnt((chess::Color)c, chess::BISHOP);
            ph += wR * cnt((chess::Color)c, chess::ROOK);
            ph += wQ * cnt((chess::Color)c, chess::QUEEN);
        }

        if (ph > maxPhase) ph = maxPhase;
        return (ph * 256 + (maxPhase / 2)) / maxPhase;
    }

    static int blend(const chess::Position& pos, PhaseScore ps) {
        const int p = phase256(pos);
        return (ps.mg * p + ps.eg * (256 - p) + 128) >> 8;
    }
};

} // namespace eval