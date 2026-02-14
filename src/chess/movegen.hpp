#pragma once

#include <vector>
#include "position.hpp"
#include "move.hpp"
#include "attacks.hpp"
#include "legality.hpp"
#include "make.hpp"

namespace chess {

void generate_pseudo_legal(const Position& pos, std::vector<Move>& out);
void generate_legal(Position& pos, std::vector<Move>& out);

} // namespace chess
