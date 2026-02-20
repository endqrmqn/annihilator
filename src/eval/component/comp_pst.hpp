#pragma once

#include "../eval_component.hpp"
#include "../../chess/position.hpp"
#include "../../chess/move.hpp"
#include "../../chess/bitboard.hpp"

namespace eval {

struct CompPST {
    void init(const chess::Position&) {}

    PhaseScore value(const chess::Position& pos, chess::Color us) const;

    void on_make_move(const chess::Position&, chess::Move) {}
    void on_unmake_move(const chess::Position&, chess::Move) {}

    MoveDelta estimate_delta(const chess::Position& pos, chess::Move m) const;

private:
    static inline chess::Square mirror_sq(chess::Square sq) {
        // flip rank: a1<->a8 etc. (0..63, a1=0)
        return sq ^ 56;
    }

    static inline int16_t mg_tbl(chess::PieceType pt, chess::Square sq) {
        // simple (placeholder) PST: center bonus; replace later with real tables
        // sq is from WHITE POV
        const int f = chess::f_of(sq);
        const int r = chess::r_of(sq);
        const int dc = (f > 3 ? f - 3 : 3 - f) + (r > 3 ? r - 3 : 3 - r); // manhattan from center
        const int center = (6 - dc); // higher near center
        switch (pt) {
            case chess::PAWN:   return int16_t(  2 * r);                 // push pawns in mg
            case chess::KNIGHT: return int16_t(  6 * center);
            case chess::BISHOP: return int16_t(  4 * center);
            case chess::ROOK:   return int16_t(  2 * (r >= 6));          // 7th rank
            case chess::QUEEN:  return int16_t(  2 * center);
            case chess::KING:   return int16_t( -6 * center);            // avoid center in mg
            default:            return 0;
        }
    }

    static inline int16_t eg_tbl(chess::PieceType pt, chess::Square sq) {
        const int f = chess::f_of(sq);
        const int r = chess::r_of(sq);
        const int dc = (f > 3 ? f - 3 : 3 - f) + (r > 3 ? r - 3 : 3 - r);
        const int center = (6 - dc);
        switch (pt) {
            case chess::PAWN:   return int16_t(  4 * r);                 // passed pawns later
            case chess::KNIGHT: return int16_t(  2 * center);
            case chess::BISHOP: return int16_t(  2 * center);
            case chess::ROOK:   return int16_t(  1 * center);
            case chess::QUEEN:  return int16_t(  1 * center);
            case chess::KING:   return int16_t(  8 * center);            // king to center in eg
            default:            return 0;
        }
    }

    static inline PhaseScore pst(chess::PieceType pt, chess::Square sq, chess::Color pc) {
        // tables stored from white POV; mirror black squares
        chess::Square s = (pc == chess::WHITE) ? sq : mirror_sq(sq);
        return PhaseScore{ mg_tbl(pt, s), eg_tbl(pt, s) };
    }

    static inline chess::PieceType captured_piece_type_at(
        const chess::Position& pos, chess::Color victim, chess::Square sq
    ) {
        const chess::Bitboard m = chess::bb_of(sq);
        for (int p = 0; p < 6; ++p) {
            if (pos.pieces[(int)victim][p] & m) return (chess::PieceType)p;
        }
        return chess::NO_PIECE_TYPE;
    }

    static inline chess::PieceType moved_piece_type_at(
        const chess::Position& pos, chess::Color mover, chess::Square sq
    ) {
        const chess::Bitboard m = chess::bb_of(sq);
        for (int p = 0; p < 6; ++p) {
            if (pos.pieces[(int)mover][p] & m) return (chess::PieceType)p;
        }
        return chess::NO_PIECE_TYPE;
    }
};

} // namespace eval