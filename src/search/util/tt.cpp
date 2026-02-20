#include "tt.hpp"

namespace search {

static constexpr int MATE_SCORE_CUTOFF = 800000; // your MATE is 900000

static inline bool is_mate_score(int s) {
    return s > MATE_SCORE_CUTOFF || s < -MATE_SCORE_CUTOFF;
}

int TranspositionTable::to_tt_score(int score, int ply) {
    if (!is_mate_score(score)) return score;
    return (score > 0) ? (score + ply) : (score - ply);
}

int TranspositionTable::from_tt_score(int score, int ply) {
    if (!is_mate_score(score)) return score;
    return (score > 0) ? (score - ply) : (score + ply);
}

static inline int age_of(std::uint8_t cur_gen, std::uint8_t entry_gen) {
    // unsigned wraparound age
    return (int)((std::uint8_t)(cur_gen - entry_gen));
}

void TranspositionTable::store(std::uint64_t key, int depth, TTBound bound,
                               int score, chess::Move best, int ply) {
    if (table_.empty()) return;

    TTEntry* base = cluster_base(key);

    // 1) If exact match exists in cluster, prefer updating it.
    for (std::size_t i = 0; i < CLUSTER_SIZE; ++i) {
        TTEntry* e = base + i;
        if (!e->matches(key)) continue;

        // Replace if deeper or entry is from older generation (i.e., stale).
        const bool stale = (e->gen != gen_);
        if (stale || depth >= e->depth) {
            e->key   = key;
            e->depth = (std::int16_t)depth;
            e->bound = bound;
            e->score = (std::int32_t)to_tt_score(score, ply);
            e->best  = best;
            e->gen   = gen_;
        } else {
            // Even if we don't replace, keep best move if we have none stored.
            if (e->best == chess::NO_MOVE && best != chess::NO_MOVE) e->best = best;
        }
        return;
    }

    // 2) If any empty slot, take it.
    for (std::size_t i = 0; i < CLUSTER_SIZE; ++i) {
        TTEntry* e = base + i;
        if (e->bound == TTBound::EMPTY) {
            e->key   = key;
            e->depth = (std::int16_t)depth;
            e->bound = bound;
            e->score = (std::int32_t)to_tt_score(score, ply);
            e->best  = best;
            e->gen   = gen_;
            return;
        }
    }

    // 3) Otherwise, choose a victim. Lower is worse.
    // Heuristic: prefer replacing old entries and shallow entries.
    // victim_value = depth - 4*age
    std::size_t victim = 0;
    int worst_value = 1'000'000;

    for (std::size_t i = 0; i < CLUSTER_SIZE; ++i) {
        TTEntry* e = base + i;
        const int age = age_of(gen_, e->gen);
        const int value = (int)e->depth - 4 * age;
        if (value < worst_value) {
            worst_value = value;
            victim = i;
        }
    }

    TTEntry* e = base + victim;

    e->key   = key;
    e->depth = (std::int16_t)depth;
    e->bound = bound;
    e->score = (std::int32_t)to_tt_score(score, ply);
    e->best  = best;
    e->gen   = gen_;
}

} // namespace search