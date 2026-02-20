#include "qsearch.hpp"
#include "move_order.hpp"

#include "../../chess/movegen.hpp"
#include "../../chess/make.hpp"

namespace search::util {

int qsearch(chess::Position& pos, eval::Evaluator& ev, int alpha, int beta) {
    const int stand = ev.eval_stm_cp(pos);
    if (stand >= beta) return beta;
    if (stand > alpha) alpha = stand;

    std::vector<chess::Move> moves;
    moves.reserve(128);
    chess::generate_legal(pos, moves);

    std::vector<ScoredMove> sm;
    sm.reserve(moves.size());
    for (auto m : moves) {
        if (!is_capture_like(m)) continue;
        sm.push_back({m, score_move(pos, ev, m)});
    }

    sort_moves(sm);

    for (auto& x : sm) {
        chess::Move m = x.m;

        chess::Undo u = chess::do_move(pos, m);
        ev.on_make_move(pos, m);

        int score = -qsearch(pos, ev, -beta, -alpha);

        ev.on_unmake_move(pos, m);
        chess::undo_move(pos, m, u);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

} // namespace search::util