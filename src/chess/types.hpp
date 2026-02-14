#pragma once

#include "cstdint"

namespace chess {

using Bitboard = uint64_t;
using Square = int;
constexpr Square NO_SQUARE = -1;

enum Color : uint8_t
{WHITE = 0, BLACK = 1}; 

inline Color operator~(Color c) { return Color(c ^ 1); }

enum PieceType {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE_TYPE};

enum Castling : uint8_t {
    WHITE_KING_SIDE  = 1 << 0,
    WHITE_QUEEN_SIDE = 1 << 1,
    BLACK_KING_SIDE  = 1 << 2,
    BLACK_QUEEN_SIDE = 1 << 3
};

//helpers
constexpr int f_of(Square sq) { return sq  & 7; }
constexpr int r_of(Square sq) { return sq >> 3; }
constexpr Square mk_sq(int file, int rank) { return (rank << 3) | file; }

//mask
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_H = FILE_A << 7;
} // namespace chess