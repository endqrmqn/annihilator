#include "comp_space.hpp"

namespace eval {

PhaseScore CompSpace::value(const chess::Position& pos, chess::Color us) const {
    const chess::Color them = ~us;

    const chess::Bitboard half = opponent_half(us);
    const chess::Bitboard usAtt = attacks_of(pos, us);
    const chess::Bitboard themAtt = attacks_of(pos, them);

    // "space": squares we control in their half that are not occupied by us and not controlled by them
    chess::Bitboard safe = usAtt & half & ~pos.occupied(us) & ~themAtt;

    const int cnt = chess::popcount(safe);

    // small bonus, mostly mg
    const int mg = 4 * cnt;
    const int eg = 1 * cnt;
    return {mg, eg};
}

MoveDelta CompSpace::estimate_delta(const chess::Position& pos, chess::Move m) const {
    MoveDelta out{};

    const chess::Color us = pos.stm;
    const chess::Square f = chess::from(m);
    const chess::Square t = chess::to(m);
    const uint32_t fl = chess::flags(m);

    const chess::PieceType moved = moved_piece_type_at(pos, us, f);
    if (moved != chess::PAWN) return out;

    // cheap heuristic: pawn advances into opponent half increase space
    const chess::Bitboard half = opponent_half(us);
    const bool into_half = (half & chess::bb_of(t)) != 0ULL;

    if (into_half && (fl == chess::QUIET_MOVE || (fl & chess::DPUSH) || (fl & chess::CAPTURE_MOVE) || (fl & chess::EP))) {
        out.delta.mg += 12;
        out.delta.eg += 2;
        out.valid = true;
    }

    return out;
}

} // namespace eval