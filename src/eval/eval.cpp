#include "eval.hpp"

#include "../chess/bitboard.hpp"
#include "../chess/types.hpp"
#include "../chess/attacks.hpp"

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

static inline chess::Bitboard occ_of(const chess::Position& pos, chess::Color c) {
    chess::Bitboard o = 0ULL;
    for (int p = 0; p < 6; ++p) o |= pos.pieces[c][p];
    return o;
}

static inline chess::Bitboard pawn_attack_map(const chess::Position& pos, chess::Color c) {
    chess::Bitboard pawns = pos.pieces[c][chess::PAWN];
    chess::Bitboard att = 0ULL;
    while (pawns) {
        chess::Square sq = chess::pop_lsb(pawns);
        att |= chess::pawn_attacks[c][sq];
    }
    return att;
}

static inline int file_of(chess::Square sq) { return chess::f_of(sq); }

// optional helper masks (if you already have FILE_A..FILE_H in types.hpp)
static constexpr chess::Bitboard FILE_MASKS[8] = {
    chess::FILE_A, chess::FILE_B, chess::FILE_C, chess::FILE_D,
    chess::FILE_E, chess::FILE_F, chess::FILE_G, chess::FILE_H
};

static int activity_side(const chess::Position& pos,
                         chess::Color us,
                         chess::Bitboard occ,
                         chess::Bitboard ourOcc,
                         chess::Bitboard enemyPawnAtt) {
    using namespace chess;

    int s = 0;

    auto score_piece = [&](PieceType pt, Square sq, Bitboard attacks) {
        Bitboard moves = attacks & ~ourOcc;
        int mob = popcount(moves);

        Bitboard safeMoves = moves & ~enemyPawnAtt;
        int safeMob = popcount(safeMoves);

        int wMob = 0, wSafe = 0;
        int badThresh = 0, badPenalty = 0;

        switch (pt) {
            case KNIGHT: wMob = 2; wSafe = 5; badThresh = 2; badPenalty = 10; break;
            case BISHOP: wMob = 2; wSafe = 5; badThresh = 3; badPenalty = 10; break;
            case ROOK:   wMob = 1; wSafe = 3; badThresh = 2; badPenalty = 8;  break;
            case QUEEN:  wMob = 0; wSafe = 2; badThresh = 3; badPenalty = 6;  break;
            default: return;
        }

        s += mob * wMob + safeMob * wSafe;

        // "no bad pieces": punish lack of safe squares
        if (safeMob < badThresh) s -= badPenalty * (badThresh - safeMob);
    };

    // knights
    Bitboard bb = pos.pieces[us][KNIGHT];
    while (bb) {
        Square sq = pop_lsb(bb);
        score_piece(KNIGHT, sq, knight_attacks[sq]);
    }

    // bishops
    bb = pos.pieces[us][BISHOP];
    while (bb) {
        Square sq = pop_lsb(bb);
        score_piece(BISHOP, sq, bishop_attacks(sq, occ));
    }

    // rooks (+ tiny open-file bonus to make them not stupid)
    bb = pos.pieces[us][ROOK];
    Bitboard ourPawns = pos.pieces[us][PAWN];
    Bitboard allPawns = pos.pieces[WHITE][PAWN] | pos.pieces[BLACK][PAWN];
    while (bb) {
        Square sq = pop_lsb(bb);
        score_piece(ROOK, sq, rook_attacks(sq, occ));

        int f = file_of(sq);
        Bitboard fileMask = FILE_MASKS[f];
        bool semiOpen = (ourPawns & fileMask) == 0ULL;
        bool open     = (allPawns & fileMask) == 0ULL;

        if (open)     s += 12;
        else if (semiOpen) s += 6;
    }

    // queens
    bb = pos.pieces[us][QUEEN];
    while (bb) {
        Square sq = pop_lsb(bb);
        score_piece(QUEEN, sq, queen_attacks(sq, occ));
    }

    return s;
}

// score from side-to-move perspective (positive = good for stm)
int evaluate(const chess::Position& pos) {
    using namespace chess;

    // material
    int w = 0, b = 0;
    for (int p = 0; p < 6; ++p) {
        w += popcount(pos.pieces[WHITE][p]) * val((PieceType)p);
        b += popcount(pos.pieces[BLACK][p]) * val((PieceType)p);
    }
    int score = (w - b);

    // occupancy + pawn control
    Bitboard wOcc = occ_of(pos, WHITE);
    Bitboard bOcc = occ_of(pos, BLACK);
    Bitboard occ  = wOcc | bOcc;

    Bitboard wPawnAtt = pawn_attack_map(pos, WHITE);
    Bitboard bPawnAtt = pawn_attack_map(pos, BLACK);

    // activity (white-good)
    int actW = activity_side(pos, WHITE, occ, wOcc, bPawnAtt);
    int actB = activity_side(pos, BLACK, occ, bOcc, wPawnAtt);
    score += (actW - actB);

    return (pos.stm == WHITE) ? score : -score;
}

} // namespace eval
