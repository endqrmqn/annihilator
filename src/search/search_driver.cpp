#include "search.hpp"
#include "search_core.hpp"

#include <vector>
#include <algorithm>

#include "../chess/movegen.hpp"
#include "../chess/make.hpp"
#include "../chess/legality.hpp"

#include "../eval/eval.hpp"
#include "util/move_order.hpp"

namespace search {

static constexpr int INF  = 1'000'000;
static constexpr int MATE = 900'000;

Result think(chess::Position& pos, const Limits& lim, int movetime_ms) {

    Result res{};

    State st{};
    st.eval.init(pos);

    st.tt.resize_mb(64);
    st.tt.new_search(); // keep your behavior (no clear unless you want it)

    // start timing ONCE
    st.start = std::chrono::steady_clock::now();
    st.time_limit_ms = movetime_ms;   // 0 => ignore time limit (assuming time_up handles this)
    st.stopped = false;

    std::vector<chess::Move> root;
    root.reserve(256);
    chess::generate_legal(pos, root);

    if (root.empty()) {
        res.best = chess::NO_MOVE;
        res.score = chess::in_check(pos, pos.stm) ? -MATE : 0;
        res.depth = 0;
        res.nodes = 0;
        res.elapsed_ms = st.elapsed_ms();
        return res;
    }

    // if TT is enabled, pull the root TT move (if any) to front for immediate benefit
    chess::Move root_tt_move = chess::NO_MOVE;
#if USE_TT
    {
        const std::uint64_t key = chess::compute_key(pos);
        if (auto* e = st.tt.probe(key)) {
            if (e->matches(key)) root_tt_move = e->best;
        }
    }
#endif

    // iterative deepening
    int prevScore = 0;

    for (int d = 1; d <= lim.depth; ++d) {
        if (st.time_up()) break;

        // aspiration window (helps once PV stabilizes)
        int alpha = -INF;
        int beta  =  INF;

        if (d >= 3) {
            const int window = 35; // centipawns
            alpha = prevScore - window;
            beta  = prevScore + window;
        }

        // Save the *intended* aspiration window; alpha will be updated during the search.
        const int asp_alpha = alpha;
        const int asp_beta  = beta;

        // order root moves (TT move gets a big boost if present)
        std::vector<search::util::ScoredMove> sm;
        sm.reserve(root.size());
        for (auto m : root) {
            int sc = search::util::score_move(pos, st.eval, m);
#if USE_TT
            if (m == root_tt_move) sc += 10'000'000;
            if (m == res.best)     sc += 5'000'000; // last iteration PV move
#endif
            sm.push_back({m, sc});
        }
        search::util::sort_moves(sm);

        chess::Move bestMove = sm[0].m;
        int bestScore = -INF;

        // search root moves
        for (auto& x : sm) {
            if (st.stopped) break;

            chess::Move m = x.m;

            chess::Undo u = chess::do_move(pos, m);
            st.eval.on_make_move(pos, m);

            int score = -negamax(st, pos, d - 1, -beta, -alpha, 1);

            st.eval.on_unmake_move(pos, m);
            chess::undo_move(pos, m, u);

            if (st.stopped) break;

            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
            if (score > alpha) alpha = score;

            // fail-soft root cutoff (optional, but helps when aspiration is tight)
            if (alpha >= beta) break;
        }

        // If aspiration failed, re-search with full window.
        // (Comparing against (alpha,beta) is wrong because alpha is updated to bestScore.)
        const bool aspiration_failed = (d >= 3) && !st.stopped &&
                                       (bestScore <= asp_alpha || bestScore >= asp_beta);

        if (aspiration_failed) {
            alpha = -INF;
            beta  =  INF;

            bestScore = -INF;
            bestMove  = sm[0].m;

            for (auto& x : sm) {
                if (st.stopped) break;

                chess::Move m = x.m;

                chess::Undo u = chess::do_move(pos, m);
                st.eval.on_make_move(pos, m);

                int score = -negamax(st, pos, d - 1, -beta, -alpha, 1);

                st.eval.on_unmake_move(pos, m);
                chess::undo_move(pos, m, u);

                if (st.stopped) break;

                if (score > bestScore) {
                    bestScore = score;
                    bestMove = m;
                }
                if (score > alpha) alpha = score;
            }
        }

        // only commit completed depths
        if (!st.stopped) {
            res.best = bestMove;
            res.score = bestScore;
            res.depth = d;
            prevScore = bestScore;

            // keep PV move first next iteration (massive practical speedup)
            auto it = std::find(root.begin(), root.end(), bestMove);
            if (it != root.end()) {
                std::rotate(root.begin(), it, it + 1);
            }

#if USE_TT
            // update root TT move hint for next iteration too (cheap + helpful)
            root_tt_move = bestMove;
#endif
        }
    }

    res.nodes = st.nodes;
    res.elapsed_ms = st.elapsed_ms();
    return res;
}

} // namespace search