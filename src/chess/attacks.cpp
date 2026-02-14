#include "attacks.hpp"

namespace chess {

Bitboard pawn_attacks[2][64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];

static inline bool on_board(int f, int r) { return f >= 0 && f < 8 && r >= 0 && r < 8; }

void init_attack_tables() {
    for (int sq = 0; sq < 64; ++sq) {
        int f = f_of(sq), r = r_of(sq);

        // pawn attacks
        pawn_attacks[WHITE][sq] = 0ULL;
        pawn_attacks[BLACK][sq] = 0ULL;

        if (on_board(f - 1, r + 1)) pawn_attacks[WHITE][sq] |= bb_of(mk_sq(f - 1, r + 1));
        if (on_board(f + 1, r + 1)) pawn_attacks[WHITE][sq] |= bb_of(mk_sq(f + 1, r + 1));
        if (on_board(f - 1, r - 1)) pawn_attacks[BLACK][sq] |= bb_of(mk_sq(f - 1, r - 1));
        if (on_board(f + 1, r - 1)) pawn_attacks[BLACK][sq] |= bb_of(mk_sq(f + 1, r - 1));

        // knight
        knight_attacks[sq] = 0ULL;
        const int kdf[8] = {+1,+2,+2,+1,-1,-2,-2,-1};
        const int kdr[8] = {+2,+1,-1,-2,-2,-1,+1,+2};
        for (int i = 0; i < 8; ++i) {
            int nf = f + kdf[i], nr = r + kdr[i];
            if (on_board(nf, nr)) knight_attacks[sq] |= bb_of(mk_sq(nf, nr));
        }

        // king
        king_attacks[sq] = 0ULL;
        for (int df = -1; df <= 1; ++df) {
            for (int dr = -1; dr <= 1; ++dr) {
                if (df == 0 && dr == 0) continue;
                int nf = f + df, nr = r + dr;
                if (on_board(nf, nr)) king_attacks[sq] |= bb_of(mk_sq(nf, nr));
            }
        }
    }
}

Bitboard bishop_attacks(Square sq, Bitboard occ) {
    Bitboard attacks = 0ULL;
    int f = f_of(sq), r = r_of(sq);

    // NE
    for (int nf = f + 1, nr = r + 1; on_board(nf, nr); ++nf, ++nr) {
        Square s = mk_sq(nf, nr);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }
    // NW
    for (int nf = f - 1, nr = r + 1; on_board(nf, nr); --nf, ++nr) {
        Square s = mk_sq(nf, nr);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }
    // SE
    for (int nf = f + 1, nr = r - 1; on_board(nf, nr); ++nf, --nr) {
        Square s = mk_sq(nf, nr);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }
    // SW
    for (int nf = f - 1, nr = r - 1; on_board(nf, nr); --nf, --nr) {
        Square s = mk_sq(nf, nr);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }

    return attacks;
}

Bitboard rook_attacks(Square sq, Bitboard occ) {
    Bitboard attacks = 0ULL;
    int f = f_of(sq), r = r_of(sq);

    // N
    for (int nr = r + 1; nr < 8; ++nr) {
        Square s = mk_sq(f, nr);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }
    // S
    for (int nr = r - 1; nr >= 0; --nr) {
        Square s = mk_sq(f, nr);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }
    // E
    for (int nf = f + 1; nf < 8; ++nf) {
        Square s = mk_sq(nf, r);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }
    // W
    for (int nf = f - 1; nf >= 0; --nf) {
        Square s = mk_sq(nf, r);
        attacks |= bb_of(s);
        if (occ & bb_of(s)) break;
    }

    return attacks;
}

} // namespace chess
