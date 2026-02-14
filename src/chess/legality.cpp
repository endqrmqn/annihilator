#include "legality.hpp"

namespace chess {

bool is_square_attacked(const Position& pos, Square sq, Color by) {
    Bitboard occ = pos.occ[OCC_BOTH];

    // pawns: attackers to sq come from opposite table
    // if by==WHITE, then a white pawn attacks sq if sq is in pawn_attacks[WHITE][pawn_from]
    // equivalent: pawn_from is in pawn_attacks[BLACK][sq] (reverse)
    if (pawn_attacks[~by][sq] & pos.pieces[by][PAWN]) return true;

    // knights
    if (knight_attacks[sq] & pos.pieces[by][KNIGHT]) return true;

    // king
    if (king_attacks[sq] & pos.pieces[by][KING]) return true;

    // bishops/queens
    Bitboard bq = pos.pieces[by][BISHOP] | pos.pieces[by][QUEEN];
    if (bishop_attacks(sq, occ) & bq) return true;

    // rooks/queens
    Bitboard rq = pos.pieces[by][ROOK] | pos.pieces[by][QUEEN];
    if (rook_attacks(sq, occ) & rq) return true;

    return false;
}

} // namespace chess
