#pragma once

#include "position.hpp"
#include "move.hpp"
#include "legality.hpp"

namespace chess {

struct Undo {
    uint8_t castling_rights;
    Square en_passant_square;
    int halfmove_clock;
    int fullmove_number;

    // capture restore
    bool captured;
    PieceType cap_pt;
    Square cap_sq;
};

Undo do_move(Position& pos, Move m);
void undo_move(Position& pos, Move m, const Undo& u);

// legality = doesn’t leave your own king in check
bool is_legal_move(Position& pos, Move m);

} // namespace chess
