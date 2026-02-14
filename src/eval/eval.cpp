#include "eval.hpp"
#include "../chess/bitboard.hpp"
#include "../chess/types.hpp"

namespace eval {

static inline int val(chess::PieceType pt) {
    switch (pt) {
        case chess::PAWN:   return 100;
        case chess::KNIGHT: return 320;
        case chess::BISHOP: return 330;
        case chess::ROOK:   return 500;
        case chess::QUEEN:  return 900;
        case chess::KING:   return 0;
        default:            return 0;
    }
}

// score from side-to-move perspective (positive = good for stm)
int evaluate(const chess::Position& pos) {
    int w = 0, b = 0;
    for (int p = 0; p < 6; ++p) {
        w += chess::popcount(pos.pieces[chess::WHITE][p]) * val((chess::PieceType)p);
        b += chess::popcount(pos.pieces[chess::BLACK][p]) * val((chess::PieceType)p);
    }
    int score = w - b;
    return (pos.stm == chess::WHITE) ? score : -score;
}

} // namespace eval
