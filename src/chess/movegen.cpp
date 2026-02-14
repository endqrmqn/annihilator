#include "movegen.hpp"

namespace chess {

static inline Square A1() { return mk_sq(0,0); }
static inline Square H1() { return mk_sq(7,0); }
static inline Square E1() { return mk_sq(4,0); }
static inline Square A8() { return mk_sq(0,7); }
static inline Square H8() { return mk_sq(7,7); }
static inline Square E8() { return mk_sq(4,7); }
static inline Square F1() { return mk_sq(5,0); }
static inline Square G1() { return mk_sq(6,0); }
static inline Square D1() { return mk_sq(3,0); }
static inline Square C1() { return mk_sq(2,0); }
static inline Square F8() { return mk_sq(5,7); }
static inline Square G8() { return mk_sq(6,7); }
static inline Square D8() { return mk_sq(3,7); }
static inline Square C8() { return mk_sq(2,7); }
static inline Square B1() { return mk_sq(1,0); }
static inline Square B8() { return mk_sq(1,7); }

static inline void push_move(std::vector<Move>& out, Square f, Square t, uint32_t fl=0, uint32_t pr=0) {
    out.push_back(make_move(f, t, fl, pr));
}

static inline void gen_pawns(const Position& pos, std::vector<Move>& out, Color us) {
    Color them = ~us;
    Bitboard pawns = pos.pieces[us][PAWN];
    Bitboard occB = pos.occ[OCC_BOTH];
    Bitboard theirOcc = pos.occ[them];

    while (pawns) {
        Square f = pop_lsb(pawns);
        int r = r_of(f);

        if (us == WHITE) {
            Square one = f + 8;
            if (one < 64 && (occB & bb_of(one)) == 0ULL) {
                // promotion?
                if (r == 6) {
                    push_move(out, f, one, PROMO, KNIGHT);
                    push_move(out, f, one, PROMO, BISHOP);
                    push_move(out, f, one, PROMO, ROOK);
                    push_move(out, f, one, PROMO, QUEEN);
                } else {
                    push_move(out, f, one, QUIET_MOVE);
                    // double push
                    if (r == 1) {
                        Square two = f + 16;
                        if ((occB & bb_of(two)) == 0ULL)
                            push_move(out, f, two, DPUSH);
                    }
                }
            }

            // captures
            Bitboard caps = pawn_attacks[WHITE][f] & theirOcc;
            while (caps) {
                Square t = pop_lsb(caps);
                if (r == 6) {
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, KNIGHT);
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, BISHOP);
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, ROOK);
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, QUEEN);
                } else {
                    push_move(out, f, t, CAPTURE_MOVE);
                }
            }

            // en passant
            if (pos.en_passant_square != NO_SQUARE) {
                Bitboard epMask = bb_of(pos.en_passant_square);
                if (pawn_attacks[WHITE][f] & epMask) {
                    push_move(out, f, pos.en_passant_square, CAPTURE_MOVE | EP);
                }
            }

        } else { // BLACK
            Square one = f - 8;
            if (one >= 0 && (occB & bb_of(one)) == 0ULL) {
                if (r == 1) {
                    push_move(out, f, one, PROMO, KNIGHT);
                    push_move(out, f, one, PROMO, BISHOP);
                    push_move(out, f, one, PROMO, ROOK);
                    push_move(out, f, one, PROMO, QUEEN);
                } else {
                    push_move(out, f, one, QUIET_MOVE);
                    if (r == 6) {
                        Square two = f - 16;
                        if ((occB & bb_of(two)) == 0ULL)
                            push_move(out, f, two, DPUSH);
                    }
                }
            }

            Bitboard caps = pawn_attacks[BLACK][f] & theirOcc;
            while (caps) {
                Square t = pop_lsb(caps);
                if (r == 1) {
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, KNIGHT);
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, BISHOP);
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, ROOK);
                    push_move(out, f, t, CAPTURE_MOVE | PROMO, QUEEN);
                } else {
                    push_move(out, f, t, CAPTURE_MOVE);
                }
            }

            if (pos.en_passant_square != NO_SQUARE) {
                Bitboard epMask = bb_of(pos.en_passant_square);
                if (pawn_attacks[BLACK][f] & epMask) {
                    push_move(out, f, pos.en_passant_square, CAPTURE_MOVE | EP);
                }
            }
        }
    }
}

static inline void gen_leapers(const Position& pos, std::vector<Move>& out, Color us, PieceType pt, const Bitboard* table) {
    Color them = ~us;
    Bitboard bb = pos.pieces[us][pt];
    Bitboard ours = pos.occ[us];
    Bitboard theirs = pos.occ[them];

    while (bb) {
        Square f = pop_lsb(bb);
        Bitboard atk = table[f] & ~ours;

        Bitboard caps = atk & theirs;
        Bitboard quiets = atk & ~theirs;

        while (quiets) {
            Square t = pop_lsb(quiets);
            push_move(out, f, t, QUIET_MOVE);
        }
        while (caps) {
            Square t = pop_lsb(caps);
            push_move(out, f, t, CAPTURE_MOVE);
        }
    }
}

static inline void gen_sliders(const Position& pos, std::vector<Move>& out, Color us, PieceType pt) {
    Color them = ~us;
    Bitboard bb = pos.pieces[us][pt];
    Bitboard ours = pos.occ[us];
    Bitboard theirs = pos.occ[them];
    Bitboard occB = pos.occ[OCC_BOTH];

    while (bb) {
        Square f = pop_lsb(bb);
        Bitboard atk = 0ULL;

        if (pt == BISHOP) atk = bishop_attacks(f, occB);
        else if (pt == ROOK) atk = rook_attacks(f, occB);
        else if (pt == QUEEN) atk = queen_attacks(f, occB);

        atk &= ~ours;

        Bitboard caps = atk & theirs;
        Bitboard quiets = atk & ~theirs;

        while (quiets) {
            Square t = pop_lsb(quiets);
            push_move(out, f, t, QUIET_MOVE);
        }
        while (caps) {
            Square t = pop_lsb(caps);
            push_move(out, f, t, CAPTURE_MOVE);
        }
    }
}

static inline void gen_castles(const Position& pos, std::vector<Move>& out, Color us) {
    // enforce “through check” here because legal-filtering alone isn’t sufficient for castling rules
    if (us == WHITE) {
        if (pos.castling_rights & WHITE_KING_SIDE) {
            if (pos.empty(F1()) && pos.empty(G1())
                && !in_check(pos, WHITE)
                && !is_square_attacked(pos, F1(), BLACK)
                && !is_square_attacked(pos, G1(), BLACK)) {
                push_move(out, E1(), G1(), CASTLE);
            }
        }
        if (pos.castling_rights & WHITE_QUEEN_SIDE) {
            if (pos.empty(D1()) && pos.empty(C1()) && pos.empty(B1())
                && !in_check(pos, WHITE)
                && !is_square_attacked(pos, D1(), BLACK)
                && !is_square_attacked(pos, C1(), BLACK)) {
                push_move(out, E1(), C1(), CASTLE);
            }
        }
    } else {
        if (pos.castling_rights & BLACK_KING_SIDE) {
            if (pos.empty(F8()) && pos.empty(G8())
                && !in_check(pos, BLACK)
                && !is_square_attacked(pos, F8(), WHITE)
                && !is_square_attacked(pos, G8(), WHITE)) {
                push_move(out, E8(), G8(), CASTLE);
            }
        }
        if (pos.castling_rights & BLACK_QUEEN_SIDE) {
            if (pos.empty(D8()) && pos.empty(C8()) && pos.empty(B8())
                && !in_check(pos, BLACK)
                && !is_square_attacked(pos, D8(), WHITE)
                && !is_square_attacked(pos, C8(), WHITE)) {
                push_move(out, E8(), C8(), CASTLE);
            }
        }
    }
}

void generate_pseudo_legal(const Position& pos, std::vector<Move>& out) {
    Color us = pos.stm;

    gen_pawns(pos, out, us);
    gen_leapers(pos, out, us, KNIGHT, knight_attacks);
    gen_sliders(pos, out, us, BISHOP);
    gen_sliders(pos, out, us, ROOK);
    gen_sliders(pos, out, us, QUEEN);
    gen_leapers(pos, out, us, KING, king_attacks);
    gen_castles(pos, out, us);
}

void generate_legal(Position& pos, std::vector<Move>& out) {
    std::vector<Move> pseudo;
    pseudo.reserve(256);
    generate_pseudo_legal(pos, pseudo);

    Color us = pos.stm;
    for (Move m : pseudo) {
        Undo u = do_move(pos, m);
        bool ok = !in_check(pos, us);
        undo_move(pos, m, u);
        if (ok) out.push_back(m);
    }
}

} // namespace chess
