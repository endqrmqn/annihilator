#pragma once

#include "../eval_component.hpp"

// chess headers (component/ is nested under eval/)
#include "../../chess/position.hpp"
#include "../../chess/move.hpp"
#include "../../chess/bitboard.hpp"

namespace eval {

struct CompMaterial {
    void init(const chess::Position&) {}

    PhaseScore value(const chess::Position& pos, chess::Color us) const;

    void on_make_move(const chess::Position&, chess::Move) {}
    void on_unmake_move(const chess::Position&, chess::Move) {}

    MoveDelta estimate_delta(const chess::Position& pos, chess::Move m) const;

private:
    static constexpr int P = 100;
    static constexpr int N = 320;
    static constexpr int B = 330;
    static constexpr int R = 500;
    static constexpr int Q = 900;

    static int piece_value(chess::PieceType pt);

    static chess::PieceType captured_piece_type_at(
        const chess::Position& pos,
        chess::Color victim,
        chess::Square sq
    );
};

} // namespace eval