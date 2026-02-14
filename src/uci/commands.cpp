#include "commands.hpp"

#include <sstream>
#include <vector>
#include <iostream>
#include <cctype>

#include "../chess/movegen.hpp"
#include "../chess/make.hpp"
#include "../chess/attacks.hpp"

#include "../search/search.hpp"


namespace uci {

static const char* STARTPOS_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

static inline std::vector<std::string> split_tokens(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> out;
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

static inline bool parse_square(const std::string& ss, chess::Square& out) {
    if (ss.size() != 2) return false;
    char file = ss[0];
    char rank = ss[1];
    if (file < 'a' || file > 'h') return false;
    if (rank < '1' || rank > '8') return false;
    int f = file - 'a';
    int r = rank - '1';
    out = chess::mk_sq(f, r);
    return true;
}

static inline chess::PieceType promo_char_to_pt(char c) {
    c = (char)std::tolower((unsigned char)c);
    switch (c) {
        case 'n': return chess::KNIGHT;
        case 'b': return chess::BISHOP;
        case 'r': return chess::ROOK;
        case 'q': return chess::QUEEN;
        default:  return chess::NO_PIECE_TYPE;
    }
}

static inline std::string square_to_str(chess::Square sq) {
    int f = chess::f_of(sq);
    int r = chess::r_of(sq);
    std::string s;
    s.push_back(char('a' + f));
    s.push_back(char('1' + r));
    return s;
}

static inline std::string move_to_uci(chess::Move m) {
    if (m == chess::NO_MOVE) return "0000";
    std::string s;
    s += square_to_str(chess::from(m));
    s += square_to_str(chess::to(m));
    if (chess::flags(m) & chess::PROMO) {
        chess::PieceType pt = (chess::PieceType)chess::promo(m);
        char pc = 'q';
        if (pt == chess::KNIGHT) pc = 'n';
        else if (pt == chess::BISHOP) pc = 'b';
        else if (pt == chess::ROOK) pc = 'r';
        else if (pt == chess::QUEEN) pc = 'q';
        s.push_back(pc);
    }
    return s;
}

// parse "e2e4" or "e7e8q" by matching against legal moves
static chess::Move parse_uci_move(chess::Position& pos, const std::string& ms) {
    if (ms.size() != 4 && ms.size() != 5) return chess::NO_MOVE;

    chess::Square f, t;
    if (!parse_square(ms.substr(0,2), f)) return chess::NO_MOVE;
    if (!parse_square(ms.substr(2,2), t)) return chess::NO_MOVE;

    chess::PieceType wantPromo = chess::NO_PIECE_TYPE;
    if (ms.size() == 5) wantPromo = promo_char_to_pt(ms[4]);

    std::vector<chess::Move> moves;
    moves.reserve(256);
    chess::generate_legal(pos, moves);

    for (auto m : moves) {
        if (chess::from(m) != f || chess::to(m) != t) continue;

        if (ms.size() == 4) {
            // accept non-promo move only
            if ((chess::flags(m) & chess::PROMO) == 0) return m;
        } else {
            // promo required
            if ((chess::flags(m) & chess::PROMO) == 0) continue;
            chess::PieceType pm = (chess::PieceType)chess::promo(m);
            if (pm == wantPromo) return m;
        }
    }

    return chess::NO_MOVE;
}

static void cmd_position(UciState& st, const std::vector<std::string>& tok) {
    // position startpos [moves ...]
    // position fen <fen...> [moves ...]
    if (tok.size() < 2) return;

    size_t i = 1;

    if (tok[i] == "startpos") {
        st.pos.set_fen(STARTPOS_FEN);
        i++;
    } else if (tok[i] == "fen") {
        i++;
        // fen is 6 fields: board stm castling ep half full
        // but we'll accept "fen" followed by everything up to "moves"
        std::string fen;
        while (i < tok.size() && tok[i] != "moves") {
            if (!fen.empty()) fen.push_back(' ');
            fen += tok[i];
            i++;
        }
        st.pos.set_fen(fen);
    } else {
        return;
    }

    // apply moves if present
    if (i < tok.size() && tok[i] == "moves") {
        i++;
        for (; i < tok.size(); ++i) {
            chess::Move m = parse_uci_move(st.pos, tok[i]);
            if (m == chess::NO_MOVE) {
                // ignore invalid move tokens rather than crashing
                continue;
            }
            chess::Undo u = chess::do_move(st.pos, m);
            (void)u; // we don't need undo in UCI forward-play
        }
    }
}

static void cmd_go(UciState& st, const std::vector<std::string>& tok) {
    int depth = 6;
    for (size_t i = 1; i + 1 < tok.size(); ++i) {
        if (tok[i] == "depth") { depth = std::stoi(tok[i+1]); break; }
    }

    search::Limits lim;
    lim.depth = depth;

    search::Result r = search::think(st.pos, lim);

    std::cout << "info depth " << depth
              << " nodes " << r.nodes
              << " score cp " << r.score
              << "\n";

    std::cout << "bestmove " << move_to_uci(r.best) << "\n";
}


bool handle_command(UciState& st, const std::string& line) {
    auto tok = split_tokens(line);
    if (tok.empty()) return true;

    const std::string& cmd = tok[0];

    if (cmd == "uci") {
        std::cout << "id name annihilator\n";
        std::cout << "id author adi\n";
        std::cout << "uciok\n";
        return true;
    }

    if (cmd == "isready") {
        std::cout << "readyok\n";
        return true;
    }

    if (cmd == "ucinewgame") {
        st.pos.set_fen(STARTPOS_FEN);
        return true;
    }

    if (cmd == "position") {
        cmd_position(st, tok);
        return true;
    }

    if (cmd == "go") {
        cmd_go(st, tok);
        return true;
    }

    if (cmd == "quit") {
        return false;
    }

    // ignore unknown commands
    return true;
}

} // namespace uci
