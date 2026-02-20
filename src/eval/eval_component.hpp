// eval/eval_component.hpp
#pragma once

#include <cstdint>
#include "../chess/types.hpp"

namespace eval {

struct PhaseScore {
    int mg = 0;
    int eg = 0;
};

inline PhaseScore operator+(PhaseScore a, PhaseScore b) {
    a.mg += b.mg;
    a.eg += b.eg;
    return a;
}

inline PhaseScore& operator+=(PhaseScore& a, PhaseScore b) {
    a.mg += b.mg;
    a.eg += b.eg;
    return a;
}

inline PhaseScore operator-(PhaseScore a, PhaseScore b) {
    a.mg -= b.mg;
    a.eg -= b.eg;
    return a;
}

inline PhaseScore operator-=(PhaseScore& a, PhaseScore b) {
    a.mg -= b.mg;
    a.eg -= b.eg;
    return a;
}

struct MoveDelta {
    PhaseScore delta{};
    bool valid = false;
    bool affects_restriction = false;
};

} // namespace eval