#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "position.hpp"
#include "movegen.hpp"
#include "make.hpp"
#include "attacks.hpp"

using namespace chess;

static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;

    std::vector<Move> moves;
    moves.reserve(256);
    generate_legal(pos, moves);

    uint64_t nodes = 0;
    for (Move m : moves) {
        Undo u = do_move(pos, m);
        nodes += perft(pos, depth - 1);
        undo_move(pos, m, u);
    }
    return nodes;
}

static void perft_divide(Position& pos, int depth) {
    std::vector<Move> moves;
    moves.reserve(256);
    generate_legal(pos, moves);

    uint64_t total = 0;
    for (Move m : moves) {
        Undo u = do_move(pos, m);
        uint64_t n = perft(pos, depth - 1);
        undo_move(pos, m, u);
        total += n;

        std::cout << from(m) << "->" << to(m) << " : " << n << "\n";
    }
    std::cout << "total: " << total << "\n";
}

static void run_test(const std::string& name,
                     const std::string& fen,
                     int depth) {
    Position p;
    p.set_fen(fen);

    std::cout << "\n== " << name << " ==\n";

    auto start = std::chrono::high_resolution_clock::now();

    uint64_t nodes = perft(p, depth);

    auto end = std::chrono::high_resolution_clock::now();
    double seconds =
        std::chrono::duration<double>(end - start).count();

    double nps = nodes / seconds;

    std::cout << "depth = " << depth << "\n";
    std::cout << "nodes = " << nodes << "\n";
    std::cout << "time  = " << seconds << " sec\n";
    std::cout << "nps   = " << (uint64_t)nps << "\n";
}


int main() {
    init_attack_tables();

    // Start position
    run_test(
        "startpos",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        7
    );

    // Kiwipete (castling/pins stress)
    run_test(
        "kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        4
    );

    // EP legality stress
    run_test(
        "ep_test",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        6
    );

    return 0;
}
