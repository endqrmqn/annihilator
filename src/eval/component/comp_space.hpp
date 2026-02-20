#pragma once

#include "../eval_component.hpp"
#include "../../chess/position.hpp"
#include "../../chess/move.hpp"
#include "../../chess/bitboard.hpp"
#include "../../chess/attacks.hpp"

namespace eval {

struct CompSpace {
    void init(const chess::Position&) {}

    PhaseScore value(const chess::Position& pos, chess::Color us) const;

    void on_make_move(const chess::Position&, chess::Move) {}
    void on_unmake_move(const chess::Position&, chess::Move) {}

    MoveDelta estimate_delta(const chess::Position& pos, chess::Move m) const;

private:
    static inline chess::Bitboard opponent_half(chess::Color us) {
        // ranks 5-8 for white, ranks 1-4 for black
        constexpr chess::Bitboard R1 = 0x00000000000000FFULL;
        constexpr chess::Bitboard R2 = 0x000000000000FF00ULL;
        constexpr chess::Bitboard R3 = 0x0000000000FF0000ULL;
        constexpr chess::Bitboard R4 = 0x00000000FF000000ULL;
        constexpr chess::Bitboard R5 = 0x000000FF00000000ULL;
        constexpr chess::Bitboard R6 = 0x0000FF0000000000ULL;
        constexpr chess::Bitboard R7 = 0x00FF000000000000ULL;
        constexpr chess::Bitboard R8 = 0xFF00000000000000ULL;

        const chess::Bitboard top = (R5|R6|R7|R8);
        const chess::Bitboard bot = (R1|R2|R3|R4);
        return (us == chess::WHITE) ? top : bot;
    }

    static inline chess::Bitboard attacks_of(const chess::Position& pos, chess::Color c) {
        chess::Bitboard occ = pos.occupied();
        chess::Bitboard a = 0ULL;

        // pawns
        {
            chess::Bitboard p = pos.pieces[(int)c][chess::PAWN];
            while (p) {
                chess::Square sq = chess::pop_lsb(p);
                a |= chess::pawn_attacks[(int)c][sq];
            }
        }

        // knights
        {
            chess::Bitboard n = pos.pieces[(int)c][chess::KNIGHT];
            while (n) {
                chess::Square sq = chess::pop_lsb(n);
                a |= chess::knight_attacks[sq];
            }
        }

        // bishops
        {
            chess::Bitboard b = pos.pieces[(int)c][chess::BISHOP];
            while (b) {
                chess::Square sq = chess::pop_lsb(b);
                a |= chess::bishop_attacks(sq, occ);
            }
        }

        // rooks
        {
            chess::Bitboard r = pos.pieces[(int)c][chess::ROOK];
            while (r) {
                chess::Square sq = chess::pop_lsb(r);
                a |= chess::rook_attacks(sq, occ);
            }
        }

        // queens
        {
            chess::Bitboard q = pos.pieces[(int)c][chess::QUEEN];
            while (q) {
                chess::Square sq = chess::pop_lsb(q);
                a |= chess::queen_attacks(sq, occ);
            }
        }

        // king
        {
            chess::Bitboard k = pos.pieces[(int)c][chess::KING];
            if (k) a |= chess::king_attacks[chess::lsb(k)];
        }

        return a;
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