#pragma once

#include <cstdint>
#include <vector>
#include <cstring>
#include <algorithm>

#include "../../chess/move.hpp"

namespace search {

enum class TTBound : std::uint8_t {
    EMPTY = 0,
    EXACT = 1,
    LOWER = 2,
    UPPER = 3
};

struct TTEntry {
    std::uint64_t key = 0;
    std::int32_t  score = 0;    // stored score (mate-adjusted)
    std::int16_t  depth = -1;   // remaining depth
    TTBound       bound = TTBound::EMPTY;
    std::uint8_t  gen = 0;
    chess::Move   best = chess::NO_MOVE;

    bool matches(std::uint64_t k) const { return bound != TTBound::EMPTY && key == k; }
};

class TranspositionTable {
public:
    static constexpr std::size_t CLUSTER_SIZE = 4;

    TranspositionTable() = default;

    void resize_mb(std::size_t mb) {
        const std::size_t bytes = std::max<std::size_t>(1, mb) * 1024ULL * 1024ULL;

        // number of clusters that fit in bytes
        std::size_t clusters = bytes / (sizeof(TTEntry) * CLUSTER_SIZE);
        if (clusters < 1) clusters = 1;

        // power-of-two clusters for fast masking
        std::size_t pow2 = 1;
        while (pow2 < clusters) pow2 <<= 1;
        clusters = pow2;

        table_.assign(clusters * CLUSTER_SIZE, TTEntry{});
        mask_ = clusters - 1;
        gen_ = 1;
    }

    void clear() {
        if (table_.empty()) return;
        std::memset(table_.data(), 0, table_.size() * sizeof(TTEntry));
    }

    void new_search() { ++gen_; if (gen_ == 0) gen_ = 1; }

    // Returns a pointer to the matching entry in the cluster, or nullptr if no match.
    TTEntry* probe(std::uint64_t key) {
        if (table_.empty()) return nullptr;
        TTEntry* base = &table_[cluster_index(key)];
        for (std::size_t i = 0; i < CLUSTER_SIZE; ++i) {
            TTEntry* e = base + i;
            if (e->matches(key)) return e;
        }
        return nullptr;
    }

    const TTEntry* probe(std::uint64_t key) const {
        if (table_.empty()) return nullptr;
        const TTEntry* base = &table_[cluster_index(key)];
        for (std::size_t i = 0; i < CLUSTER_SIZE; ++i) {
            const TTEntry* e = base + i;
            if (e->matches(key)) return e;
        }
        return nullptr;
    }

    void store(std::uint64_t key, int depth, TTBound bound, int score, chess::Move best, int ply);

    static int to_tt_score(int score, int ply);
    static int from_tt_score(int score, int ply);

private:
    std::vector<TTEntry> table_{};
    std::size_t mask_ = 0;      // mask over clusters
    std::uint8_t gen_ = 1;

    std::size_t cluster_index(std::uint64_t key) const {
        // cluster id in [0, mask_], then multiply by CLUSTER_SIZE
        return ((std::size_t)key & mask_) * CLUSTER_SIZE;
    }

    TTEntry* cluster_base(std::uint64_t key) { return &table_[cluster_index(key)]; }
    const TTEntry* cluster_base(std::uint64_t key) const { return &table_[cluster_index(key)]; }
};

} // namespace search