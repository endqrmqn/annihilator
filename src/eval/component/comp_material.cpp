#include "comp_material.hpp"

namespace eval {

int CompMaterial::piece_value(chess::PieceType pt) {
    switch (pt) {
        case chess::PAWN:   return P;
        case chess::KNIGHT: return N;
        case chess::BISHOP: return B;
        case chess::ROOK:   return R;
        case chess::QUEEN:  return Q;
        default:            return 0;
    }
}

chess::PieceType CompMaterial::captured_piece_type_at(
    const chess::Position& pos,
    chess::Color victim,
    chess::Square sq
) {
    const chess::Bitboard m = chess::bb_of(sq);
    for (int p = 0; p < 6; ++p) {
        if (pos.pieces[(int)victim][p] & m) return (chess::PieceType)p;
    }
    return chess::NO_PIECE_TYPE;
}

PhaseScore CompMaterial::value(const chess::Position& pos, chess::Color us) const {
    const chess::Color them = ~us;

    int usMat = 0;
    int themMat = 0;

    // PAWN..QUEEN only; kings ignored
    for (int p = chess::PAWN; p <= chess::QUEEN; ++p) {
        const auto pt = (chess::PieceType)p;
        const int v = piece_value(pt);

        usMat   += v * chess::popcount(pos.pieces[(int)us][p]);
        themMat += v * chess::popcount(pos.pieces[(int)them][p]);
    }

    const int s = usMat - themMat;
    return {s, s};
}

MoveDelta CompMaterial::estimate_delta(const chess::Position& pos, chess::Move m) const {
    MoveDelta out{};

    const chess::Square f = chess::from(m);
    const chess::Square t = chess::to(m);
    const uint32_t fl = chess::flags(m);

    const chess::Color us = pos.stm;
    const chess::Color them = ~us;

    // captures
    if (fl & chess::EP) {
        // en-passant is always capturing a pawn
        out.delta.mg += P;
        out.delta.eg += P;
        out.valid = true;
    } else if (fl & chess::CAPTURE_MOVE) {
        const chess::PieceType cpt = captured_piece_type_at(pos, them, t);
        const int v = piece_value(cpt);
        if (v != 0) {
            out.delta.mg += v;
            out.delta.eg += v;
            out.valid = true;
        }
    }

    // promotions
    if (fl & chess::PROMO) {
        // your make.cpp assumes promo(m) is PieceType (KNIGHT..QUEEN)
        const auto newpt = (chess::PieceType)chess::promo(m);
        const int v = piece_value(newpt) - P;
        if (v != 0) {
            out.delta.mg += v;
            out.delta.eg += v;
            out.valid = true;
        }
    }

    (void)f; // kept in case you later want special-casing
    return out;
}

} // namespace eval