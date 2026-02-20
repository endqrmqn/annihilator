#pragma once

#include "../../chess/position.hpp"
#include "../../chess/move.hpp"
#include "../../eval/eval.hpp"

namespace search::util {

// captures-only quiescence
int qsearch(chess::Position& pos, eval::Evaluator& ev, int alpha, int beta);

} // namespace search::util