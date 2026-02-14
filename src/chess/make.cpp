#include "make.hpp"

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

static inline void remove_piece(Position& pos, Color c, PieceType pt, Square sq) {
    pos.pieces[c][pt] &= ~bb_of(sq);
}
static inline void add_piece(Position& pos, Color c, PieceType pt, Square sq) {
    pos.pieces[c][pt] |= bb_of(sq);
}

static inline PieceType moving_piece_type(const Position& pos, Color c, Square fromSq) {
    Bitboard m = bb_of(fromSq);
    for (int p = 0; p < 6; ++p) if (pos.pieces[c][p] & m) return (PieceType)p;
    return NO_PIECE_TYPE;
}

static inline PieceType captured_piece_type(const Position& pos, Color c, Square toSq) {
    Bitboard m = bb_of(toSq);
    for (int p = 0; p < 6; ++p) if (pos.pieces[c][p] & m) return (PieceType)p;
    return NO_PIECE_TYPE;
}

static inline void update_castling_on_move(Position& pos, Color c, PieceType pt, Square fromSq) {
    // king move nukes both sides
    if (pt == KING) {
        if (c == WHITE) pos.castling_rights &= ~(WHITE_KING_SIDE | WHITE_QUEEN_SIDE);
        else            pos.castling_rights &= ~(BLACK_KING_SIDE | BLACK_QUEEN_SIDE);
    }
    // rook move from corner nukes that side
    if (pt == ROOK) {
        if (fromSq == H1()) pos.castling_rights &= ~WHITE_KING_SIDE;
        if (fromSq == A1()) pos.castling_rights &= ~WHITE_QUEEN_SIDE;
        if (fromSq == H8()) pos.castling_rights &= ~BLACK_KING_SIDE;
        if (fromSq == A8()) pos.castling_rights &= ~BLACK_QUEEN_SIDE;
    }
}

static inline void update_castling_on_capture(Position& pos, Color victim, Square capSq) {
    // capturing rook on corner nukes victim rights
    if (capSq == H1()) pos.castling_rights &= ~WHITE_KING_SIDE;
    if (capSq == A1()) pos.castling_rights &= ~WHITE_QUEEN_SIDE;
    if (capSq == H8()) pos.castling_rights &= ~BLACK_KING_SIDE;
    if (capSq == A8()) pos.castling_rights &= ~BLACK_QUEEN_SIDE;
}

Undo do_move(Position& pos, Move m) {
    Undo u;
    u.castling_rights = pos.castling_rights;
    u.en_passant_square = pos.en_passant_square;
    u.halfmove_clock = pos.halfmove_clock;
    u.fullmove_number = pos.fullmove_number;
    u.captured = false;
    u.cap_pt = NO_PIECE_TYPE;
    u.cap_sq = NO_SQUARE;

    const Square f = from(m);
    const Square t = to(m);
    const uint32_t fl = flags(m);
    const uint32_t pr = promo(m);

    Color us = pos.stm;
    Color them = ~us;

    // clear EP by default; set later if DPUSH
    pos.en_passant_square = NO_SQUARE;

    // find moving piece type
    PieceType pt = moving_piece_type(pos, us, f);

    // capture handling
    if (fl & EP) {
        // EP capture: target square is empty; captured pawn is behind it
        u.captured = true;
        u.cap_pt = PAWN;
        u.cap_sq = (us == WHITE) ? (t - 8) : (t + 8);
        remove_piece(pos, them, PAWN, u.cap_sq);
        update_castling_on_capture(pos, them, u.cap_sq); // harmless, but ok
        pos.halfmove_clock = 0;
    } else if (fl & CAPTURE_MOVE) {
        PieceType cpt = captured_piece_type(pos, them, t);
        u.captured = true;
        u.cap_pt = cpt;
        u.cap_sq = t;
        if (cpt != NO_PIECE_TYPE) remove_piece(pos, them, cpt, t);
        update_castling_on_capture(pos, them, t);
        pos.halfmove_clock = 0;
    }

    // pawn move resets halfmove
    if (pt == PAWN) pos.halfmove_clock = 0;
    else if (!(fl & CAPTURE_MOVE) && !(fl & EP)) pos.halfmove_clock += 1;

    // move piece (promotion/castling special)
    remove_piece(pos, us, pt, f);

    if (fl & CASTLE) {
        // king lands on t, rook moves too
        add_piece(pos, us, KING, t);

        if (us == WHITE) {
            if (t == G1()) { // king side
                remove_piece(pos, WHITE, ROOK, H1());
                add_piece(pos, WHITE, ROOK, F1());
            } else { // queen side -> C1
                remove_piece(pos, WHITE, ROOK, A1());
                add_piece(pos, WHITE, ROOK, D1());
            }
        } else {
            if (t == G8()) {
                remove_piece(pos, BLACK, ROOK, H8());
                add_piece(pos, BLACK, ROOK, F8());
            } else { // C8
                remove_piece(pos, BLACK, ROOK, A8());
                add_piece(pos, BLACK, ROOK, D8());
            }
        }

        // castling always kills your castling rights
        update_castling_on_move(pos, us, KING, f);
    }
    else if (fl & PROMO) {
        // promo field: we assume pr is PieceType (KNIGHT..QUEEN). you can enforce.
        PieceType newpt = (PieceType)pr;
        add_piece(pos, us, newpt, t);
    }
    else {
        // normal move
        add_piece(pos, us, pt, t);

        if (fl & DPUSH) {
            // set EP square (the square jumped over)
            pos.en_passant_square = (us == WHITE) ? (f + 8) : (f - 8);
        }

        update_castling_on_move(pos, us, pt, f);
    }

    // update move number / side
    if (us == BLACK) pos.fullmove_number += 1;
    pos.stm = them;

    pos.update_occ();
    return u;
}

void undo_move(Position& pos, Move m, const Undo& u) {
    const Square f = from(m);
    const Square t = to(m);
    const uint32_t fl = flags(m);
    const uint32_t pr = promo(m);

    // restore clocks/rights/ep/stm/fullmove
    pos.stm = ~pos.stm;
    pos.castling_rights = u.castling_rights;
    pos.en_passant_square = u.en_passant_square;
    pos.halfmove_clock = u.halfmove_clock;
    pos.fullmove_number = u.fullmove_number;

    Color us = pos.stm;
    Color them = ~us;

    // undo piece movement
    if (fl & CASTLE) {
        // move king back, restore rook
        remove_piece(pos, us, KING, t);
        add_piece(pos, us, KING, f);

        if (us == WHITE) {
            if (t == G1()) {
                remove_piece(pos, WHITE, ROOK, F1());
                add_piece(pos, WHITE, ROOK, H1());
            } else {
                remove_piece(pos, WHITE, ROOK, D1());
                add_piece(pos, WHITE, ROOK, A1());
            }
        } else {
            if (t == G8()) {
                remove_piece(pos, BLACK, ROOK, F8());
                add_piece(pos, BLACK, ROOK, H8());
            } else {
                remove_piece(pos, BLACK, ROOK, D8());
                add_piece(pos, BLACK, ROOK, A8());
            }
        }
    }
    else if (fl & PROMO) {
        // remove promoted piece at t, put pawn back at f
        PieceType newpt = (PieceType)pr;
        remove_piece(pos, us, newpt, t);
        add_piece(pos, us, PAWN, f);
    }
    else {
        // normal piece moved back
        PieceType pt = moving_piece_type(pos, us, t);
        remove_piece(pos, us, pt, t);
        add_piece(pos, us, pt, f);
    }

    // restore captured piece if any
    if (u.captured && u.cap_sq != NO_SQUARE && u.cap_pt != NO_PIECE_TYPE) {
        add_piece(pos, them, u.cap_pt, u.cap_sq);
    }

    pos.update_occ();
}

bool is_legal_move(Position& pos, Move m) {
    Color us = pos.stm;
    Undo u = do_move(pos, m);
    bool ok = !in_check(pos, us);
    undo_move(pos, m, u);
    return ok;
}

} // namespace chess
