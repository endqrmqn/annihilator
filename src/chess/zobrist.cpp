#include "zobrist.hpp"

namespace chess {

Zobrist ZB;

// Ensure Zobrist tables are initialized before any compute_key() calls.
// (Forgetting to call ZB.init() makes every hash key 0, which destroys TT quality.)
struct ZobristBoot {
    ZobristBoot() { ZB.init(); }
};
static ZobristBoot ZOBRIST_BOOT;

static inline std::uint64_t splitmix64(std::uint64_t& x) {
    std::uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

void Zobrist::init(std::uint64_t seed) {
    std::uint64_t x = seed;
    for (int c = 0; c < 2; ++c)
        for (int p = 0; p < 6; ++p)
            for (int s = 0; s < 64; ++s)
                piece[c][p][s] = splitmix64(x);

    for (int i = 0; i < 16; ++i) castling[i] = splitmix64(x);
    for (int i = 0; i < 9;  ++i) ep_file[i] = splitmix64(x);
    side = splitmix64(x);
}

std::uint64_t compute_key(const Position& pos) {
    std::uint64_t k = 0;

    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 6; ++p) {
            Bitboard bb = pos.pieces[c][p];
            while (bb) {
                Square s = pop_lsb(bb);
                k ^= ZB.piece[c][p][s];
            }
        }
    }

    if (pos.stm == BLACK) k ^= ZB.side;

    // castling_rights is 4 bits max, map to 0..15
    k ^= ZB.castling[pos.castling_rights & 15];

    // EP file (0..7) else 8
    int ef = 8;
    if (pos.en_passant_square != NO_SQUARE) ef = f_of(pos.en_passant_square);
    k ^= ZB.ep_file[ef];

    return k;
}

} // namespace chess