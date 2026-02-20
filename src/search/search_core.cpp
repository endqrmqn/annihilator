#ifndef USE_TT
#define USE_TT 1
#endif

#include "search_core.hpp"

#include <vector>
#include <algorithm>

#include "../chess/movegen.hpp"
#include "../chess/make.hpp"
#include "../chess/legality.hpp"

#include "util/qsearch.hpp"
#include "util/move_order.hpp"
#include "util/extensions.hpp"
#include "util/lmr_nullmove.hpp"

namespace search {

static constexpr int INF  = 1'000'000;
static constexpr int MATE = 900'000;

int negamax(State& st, chess::Position& pos, int depth, int alpha, int beta, int ply) {
    if (st.time_up()) return 0;
    st.nodes++;

    const int alpha0 = alpha;

    chess::Move tt_move = chess::NO_MOVE;

#if USE_TT
    const std::uint64_t key = chess::compute_key(pos);

    if (auto* e = st.tt.probe(key)) {
        if (e->matches(key) && e->depth >= depth) {
            tt_move = e->best;
            const int ttScore = TranspositionTable::from_tt_score(e->score, ply);

            if (e->bound == TTBound::EXACT) return ttScore;
            if (e->bound == TTBound::LOWER) alpha = std::max(alpha, ttScore);
            else if (e->bound == TTBound::UPPER) beta = std::min(beta, ttScore);

            if (alpha >= beta) return ttScore;
        }
    }
#endif

    if (depth <= 0) {
        return search::util::qsearch(pos, st.eval, alpha, beta);
    }

    std::vector<chess::Move> moves;
    moves.reserve(256);
    chess::generate_legal(pos, moves);

    if (moves.empty()) {
        // mate distance should depend on ply for consistent mate scoring + TT mate shifting
        if (chess::in_check(pos, pos.stm)) return -MATE + ply;
        return 0;
    }

    std::vector<search::util::ScoredMove> sm;
    sm.reserve(moves.size());
    for (auto m : moves) {
        int sc = search::util::score_move(pos, st.eval, m);
#if USE_TT
        if (m == tt_move) sc += 10'000'000; // TT move first
#endif
        sm.push_back({m, sc});
    }
    search::util::sort_moves(sm);

    chess::Move bestMove = chess::NO_MOVE;

    int idx = 0;
    for (auto& x : sm) {
        if (st.stopped) break;

        chess::Move m = x.m;

        const bool cap = search::util::is_capture_like(m);
        const int ext  = search::util::extension_for(m);
        const int red  = search::util::lmr_reduction(depth, idx, cap);

        chess::Undo u = chess::do_move(pos, m);
        st.eval.on_make_move(pos, m);

        int score;
        if (red > 0) {
            score = -negamax(st, pos, depth - 1 - red + ext, -beta, -alpha, ply + 1);
            if (score > alpha) {
                score = -negamax(st, pos, depth - 1 + ext, -beta, -alpha, ply + 1);
            }
        } else {
            score = -negamax(st, pos, depth - 1 + ext, -beta, -alpha, ply + 1);
        }

        st.eval.on_unmake_move(pos, m);
        chess::undo_move(pos, m, u);

        if (score >= beta) {
#if USE_TT
            // store the *actual* cutoff score (more informative than storing beta)
            st.tt.store(key, depth, TTBound::LOWER, score, m, ply);
#endif
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            bestMove = m;
        }

        ++idx;
    }

#if USE_TT
    const TTBound bound = (alpha > alpha0) ? TTBound::EXACT : TTBound::UPPER;
    st.tt.store(key, depth, bound, alpha, bestMove, ply);
#endif

    return alpha;
}

} // namespace search