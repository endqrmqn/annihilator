// eval/eval_aggregator.hpp
#pragma once

#include <tuple>
#include <utility>

#include "../chess/position.hpp"
#include "../chess/move.hpp"

#include "eval_component.hpp"

namespace eval {

template <class Tuple, class F, std::size_t... I>
inline void tuple_for_each_impl(Tuple& t, F&& f, std::index_sequence<I...>) {
    (f(std::get<I>(t)), ...);
}

template <class Tuple, class F>
inline void tuple_for_each(Tuple& t, F&& f) {
    tuple_for_each_impl(
        t,
        std::forward<F>(f),
        std::make_index_sequence<std::tuple_size_v<Tuple>>{}
    );
}

template <class... Components>
class Aggregator {
public:
    void init(const chess::Position& pos) {
        tuple_for_each(comps_, [&](auto& c) { c.init(pos); });
    }

    PhaseScore value(const chess::Position& pos, chess::Color us) const {
        PhaseScore out{};
        tuple_for_each(
            const_cast<std::tuple<Components...>&>(comps_),
            [&](auto& c) { out += c.value(pos, us); }
        );
        return out;
    }

    void on_make_move(const chess::Position& pos, chess::Move m) {
        tuple_for_each(comps_, [&](auto& c) { c.on_make_move(pos, m); });
    }

    void on_unmake_move(const chess::Position& pos, chess::Move m) {
        tuple_for_each(comps_, [&](auto& c) { c.on_unmake_move(pos, m); });
    }

    MoveDelta estimate_delta(const chess::Position& pos, chess::Move m) const {
        MoveDelta out{};
        bool any = false;

        tuple_for_each(
            const_cast<std::tuple<Components...>&>(comps_),
            [&](auto& c) {
                MoveDelta d = c.estimate_delta(pos, m);
                if (d.valid) {
                    any = true;
                    out.delta += d.delta;
                    out.affects_restriction |= d.affects_restriction;
                }
            }
        );

        out.valid = any;
        return out;
    }

private:
    std::tuple<Components...> comps_{};
};

} // namespace eval