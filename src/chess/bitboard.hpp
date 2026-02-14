#pragma once

#include "types.hpp"

namespace chess {
inline int popcount(Bitboard b) {
    return __builtin_popcountll(b);
}

inline Square lsb(Bitboard b) {
    return __builtin_ctzll(b);
}

inline Square msb(Bitboard b) {
    return 63 - lsb(b);
}

inline Square pop_lsb(Bitboard &b) {
    Square sq = lsb(b);
    b &= b - 1;
    return sq;
}

inline Square pop_msb(Bitboard &b) {
    Square sq = msb(b);
    b &= ~(1ULL << sq);
    return sq;
}

constexpr Bitboard bb_of(Square sq) {
    return 1ULL << sq;
}

inline Bitboard n_shift(Bitboard b) {
    return b << 8;
}
inline Bitboard s_shift(Bitboard b) {
    return b >> 8;
}
inline Bitboard e_shift(Bitboard b) {
    return (b & ~FILE_H) << 1;
}
inline Bitboard w_shift(Bitboard b) {
    return (b & ~FILE_A) >> 1;
}
} //namespace chess