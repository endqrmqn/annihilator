#include "comp_prophylaxis.hpp"

namespace eval {

int CompProphylaxis::legal_moves_for_side(const chess::Position& pos, chess::Color side) {
    chess::Position tmp = pos;
    tmp.stm = side;

    // generate_legal mutates Position (does do_move/undo internally), hence tmp is non-const.
    std::vector<chess::Move> moves;
    moves.reserve(64);
    chess::generate_legal(tmp, moves);
    return (int)moves.size();
}

void CompProphylaxis::recompute(const chess::Position& pos) {
    legal_ct[(int)chess::WHITE] = legal_moves_for_side(pos, chess::WHITE);
    legal_ct[(int)chess::BLACK] = legal_moves_for_side(pos, chess::BLACK);
}

PhaseScore CompProphylaxis::value(const chess::Position& pos, chess::Color us) const {
    (void)pos;

    const chess::Color them = ~us;

    // fewer opponent legal moves => higher restriction score for us
    // baseline keeps it from overrewarding normal middlegame mobility
    const int opp = legal_ct[(int)them];
    const int base = 30;

    const int diff = base - opp;          // positive if opp < base
    const int mg = 6 * diff;
    const int eg = 2 * diff;
    return {mg, eg};
}

MoveDelta CompProphylaxis::estimate_delta(const chess::Position& pos, chess::Move m) const {
    MoveDelta out{};

    const chess::Color us = pos.stm;
    const chess::Color them = ~us;

    const uint32_t fl = chess::flags(m);

    // cheap “restriction-affecting” tagging:
    // - captures tend to reduce opponent resources / squares
    // - checks force a reply (temporarily restrict)
    bool interesting = (fl & chess::CAPTURE_MOVE) || (fl & chess::EP) || (fl & chess::PROMO);

    // check detection: do move on a copy and see if opponent is in check
    // (still fairly cheap; if you want ultra-cheap later, we can do attack-based check test)
    chess::Position tmp = pos;
    chess::Undo u = chess::do_move(tmp, m);
    const bool gives_check = chess::in_check(tmp, them);
    chess::undo_move(tmp, m, u);

    if (gives_check) interesting = true;

    if (!interesting) return out;

    out.valid = true;
    out.affects_restriction = true;

    // small generic bonus; the real restriction is from full eval / ordering later
    out.delta.mg += gives_check ? 18 : 10;
    out.delta.eg += gives_check ? 6  : 3;

    return out;
}

} // namespace eval