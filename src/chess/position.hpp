#pragma once

#include <string>
#include "types.hpp"
#include "bitboard.hpp"

namespace chess {

enum OccIndex : int { OCC_WHITE = 0, OCC_BLACK = 1, OCC_BOTH = 2 };

struct Position {
    // pieces[color][pieceType] where pieceType is PAWN..KING (0..5)
    Bitboard pieces[2][6]{};

    // occupancies
    Bitboard occ[3]{}; // [white, black, both]

    Color stm = WHITE;
    uint8_t castling_rights = 0; // bitmask using Castling bits
    Square en_passant_square = NO_SQUARE;

    // optional bookkeeping (useful for perft/fen/50-move later)
    int halfmove_clock = 0;
    int fullmove_number = 1;

    // ---------------- core maintenance ----------------

    inline void clear() {
        for (int c = 0; c < 2; ++c)
            for (int p = 0; p < 6; ++p)
                pieces[c][p] = 0ULL;

        occ[OCC_WHITE] = occ[OCC_BLACK] = occ[OCC_BOTH] = 0ULL;

        stm = WHITE;
        castling_rights = 0;
        en_passant_square = NO_SQUARE;
        halfmove_clock = 0;
        fullmove_number = 1;
    }

    inline void update_occ() {
        occ[OCC_WHITE] = 0ULL;
        occ[OCC_BLACK] = 0ULL;

        for (int p = 0; p < 6; ++p) {
            occ[OCC_WHITE] |= pieces[WHITE][p];
            occ[OCC_BLACK] |= pieces[BLACK][p];
        }

        occ[OCC_BOTH] = occ[OCC_WHITE] | occ[OCC_BLACK];
    }

    inline Bitboard occupied() const { return occ[OCC_BOTH]; }
    inline Bitboard occupied(Color c) const { return occ[(int)c]; }

    // ---------------- square queries ----------------

    inline bool empty(Square sq) const {
        return (occ[OCC_BOTH] & bb_of(sq)) == 0ULL;
    }

    inline bool occupied_by(Color c, Square sq) const {
        return (occ[(int)c] & bb_of(sq)) != 0ULL;
    }

    // returns NO_PIECE_TYPE if empty. sets outColor if found.
    inline PieceType piece_on(Square sq, Color &outColor) const {
        Bitboard m = bb_of(sq);

        // quick reject
        if ((occ[OCC_BOTH] & m) == 0ULL) {
            outColor = WHITE; // arbitrary
            return NO_PIECE_TYPE;
        }

        for (int c = 0; c < 2; ++c) {
            if ((occ[c] & m) == 0ULL) continue;
            for (int p = 0; p < 6; ++p) {
                if (pieces[c][p] & m) {
                    outColor = (Color)c;
                    return (PieceType)p;
                }
            }
        }

        outColor = WHITE; // arbitrary
        return NO_PIECE_TYPE;
    }

    inline Square king_square(Color c) const {
        Bitboard k = pieces[c][KING];
        if (!k) return NO_SQUARE;
        return lsb(k); // you already have lsb(Bitboard)->Square index
    }

    // ---------------- castling helpers ----------------

    inline bool can_castle(Castling cr) const { return (castling_rights & cr) != 0; }
    inline void disable_castle(Castling cr) { castling_rights &= ~uint8_t(cr); }

    // ---------------- FEN (optional stubs) ----------------
    
    bool set_fen(const std::string& fen);
    std::string fen() const;
};

} // namespace chess
