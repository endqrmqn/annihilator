#include "search.hpp"

#include <vector>
#include <algorithm>

#include "../chess/movegen.hpp"
#include "../chess/make.hpp"
#include "../chess/legality.hpp"
#include "../eval/eval.hpp"

namespace search {

static constexpr int INF  = 1'000'000;
static constexpr int MATE = 900'000;

struct State {
    std::uint64_t nodes = 0;
};

// -------------------- root ordering / tie-break --------------------

static inline int root_heuristic(chess::Move m) {
    int s = 0;

    // hard priorities
    if (chess::flags(m) & chess::PROMO)        s += 20000;
    if (chess::flags(m) & chess::CAPTURE_MOVE) s += 10000;
    if (chess::flags(m) & chess::CASTLE)       s += 5000;

    // center-ish destination bonus (c3..f6 box)
    int to = chess::to(m);
    int f = chess::f_of(to), r = chess::r_of(to);
    if (f >= 2 && f <= 5 && r >= 2 && r <= 5) s += 50;

    // tiny penalty for a/h-file shuffles
    int from = chess::from(m);
    int ff = chess::f_of(from);
    if (ff == 0 || ff == 7) s -= 5;

    return s;
}

static inline void sort_root(std::vector<chess::Move>& root) {
    std::stable_sort(root.begin(), root.end(),
        [](chess::Move a, chess::Move b) {
            return root_heuristic(a) > root_heuristic(b);
        });
}

// -------------------- quiescence --------------------

static inline void generate_legal_captures(chess::Position& pos, std::vector<chess::Move>& out) {
    std::vector<chess::Move> tmp;
    tmp.reserve(256);
    chess::generate_legal(pos, tmp);

    out.clear();
    out.reserve(tmp.size());
    for (auto m : tmp) {
        if (chess::flags(m) & chess::CAPTURE_MOVE) out.push_back(m);
        else if (chess::flags(m) & chess::EP) out.push_back(m); // EP is a capture
    }
}

static int quiescence(State& st, chess::Position& pos, int alpha, int beta) {
    st.nodes++;

    int stand_pat = eval::evaluate(pos);

    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    std::vector<chess::Move> caps;
    caps.reserve(64);
    generate_legal_captures(pos, caps);

    // basic ordering: captures first (already all captures), promotions, etc.
    std::stable_sort(caps.begin(), caps.end(),
        [](chess::Move a, chess::Move b) {
            // promo > capture (they're both usually captures) - small ordering help
            return (chess::flags(a) & chess::PROMO) > (chess::flags(b) & chess::PROMO);
        });

    for (auto m : caps) {
        chess::Undo u = chess::do_move(pos, m);
        int score = -quiescence(st, pos, -beta, -alpha);
        chess::undo_move(pos, m, u);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// -------------------- negamax --------------------

static int negamax(State& st, chess::Position& pos, int depth, int alpha, int beta) {
    st.nodes++;

    if (depth == 0) {
        // horizon: resolve tactical noise
        return quiescence(st, pos, alpha, beta);
    }

    std::vector<chess::Move> moves;
    moves.reserve(256);
    chess::generate_legal(pos, moves);

    if (moves.empty()) {
        // mate or stalemate
        if (chess::in_check(pos, pos.stm)) return -MATE + 1;
        return 0;
    }

    // tiny move ordering: captures first
    std::stable_sort(moves.begin(), moves.end(),
        [](chess::Move a, chess::Move b) {
            return (chess::flags(a) & chess::CAPTURE_MOVE) > (chess::flags(b) & chess::CAPTURE_MOVE);
        });

    for (auto m : moves) {
        chess::Undo u = chess::do_move(pos, m);
        int score = -negamax(st, pos, depth - 1, -beta, -alpha);
        chess::undo_move(pos, m, u);

        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }

    return alpha;
}

// -------------------- think --------------------

Result think(chess::Position& pos, const Limits& lim) {
    State st;
    Result res;
    res.best = chess::NO_MOVE;
    res.score = -INF;

    std::vector<chess::Move> root;
    root.reserve(256);
    chess::generate_legal(pos, root);

    if (root.empty()) {
        res.best = chess::NO_MOVE;
        res.score = chess::in_check(pos, pos.stm) ? -MATE : 0;
        res.nodes = 0;
        return res;
    }

    // initial root ordering
    sort_root(root);

    // iterative deepening
    for (int d = 1; d <= lim.depth; ++d) {
        int alpha = -INF;
        int beta  = INF;

        chess::Move bestMove = root[0];
        int bestScore = -INF;

        // keep root heuristic ordering each iteration (do NOT overwrite with capture-only sort)
        sort_root(root);

        for (auto m : root) {
            chess::Undo u = chess::do_move(pos, m);
            int score = -negamax(st, pos, d - 1, -beta, -alpha);
            chess::undo_move(pos, m, u);

            if (score > bestScore || (score == bestScore && root_heuristic(m) > root_heuristic(bestMove))) {
                bestScore = score;
                bestMove = m;
            }

            if (score > alpha) alpha = score;
        }

        res.best = bestMove;
        res.score = bestScore;
    }

    res.nodes = st.nodes;
    return res;
}

} // namespace search
