#include "comp_pst.hpp"

namespace eval {

PhaseScore CompPST::value(const chess::Position& pos, chess::Color us) const {
    const chess::Color them = ~us;

    PhaseScore usS{}, themS{};

    for (int pt = 0; pt < 6; ++pt) { // includes king
        chess::Bitboard b = pos.pieces[(int)us][pt];
        while (b) {
            chess::Square sq = chess::pop_lsb(b);
            usS += pst((chess::PieceType)pt, sq, us);
        }
        b = pos.pieces[(int)them][pt];
        while (b) {
            chess::Square sq = chess::pop_lsb(b);
            themS += pst((chess::PieceType)pt, sq, them);
        }
    }

    return usS - themS;
}

MoveDelta CompPST::estimate_delta(const chess::Position& pos, chess::Move m) const {
    MoveDelta out{};

    const chess::Color us = pos.stm;
    const chess::Color them = ~us;

    const chess::Square f = chess::from(m);
    const chess::Square t = chess::to(m);
    const uint32_t fl = chess::flags(m);

    const chess::PieceType moved = moved_piece_type_at(pos, us, f);
    if (moved == chess::NO_PIECE_TYPE) return out;

    // remove moved piece from f, add to t (or promo piece to t)
    if (fl & chess::PROMO) {
        const chess::PieceType newpt = (chess::PieceType)chess::promo(m);
        out.delta += pst(newpt, t, us);
        out.delta -= pst(chess::PAWN, f, us);
        out.valid = true;
    } else {
        out.delta += pst(moved, t, us);
        out.delta -= pst(moved, f, us);
        out.valid = true;
    }

    // captures improve us because opponent PST disappears
    if (fl & chess::EP) {
        // captured pawn is behind target square
        const chess::Square cap_sq = (us == chess::WHITE) ? (t - 8) : (t + 8);
        out.delta += pst(chess::PAWN, cap_sq, them);
        out.valid = true;
    } else if (fl & chess::CAPTURE_MOVE) {
        const chess::PieceType cpt = captured_piece_type_at(pos, them, t);
        if (cpt != chess::NO_PIECE_TYPE) {
            out.delta += pst(cpt, t, them);
            out.valid = true;
        }
    }

    return out;
}

} // namespace eval