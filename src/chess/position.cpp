#include "position.hpp"
#include <sstream>
#include <cctype>

namespace chess {

static inline char piece_to_char(Color c, PieceType pt) {
    char ch = '?';
    switch (pt) {
        case PAWN:   ch = 'p'; break;
        case KNIGHT: ch = 'n'; break;
        case BISHOP: ch = 'b'; break;
        case ROOK:   ch = 'r'; break;
        case QUEEN:  ch = 'q'; break;
        case KING:   ch = 'k'; break;
        default:     ch = '?'; break;
    }
    if (c == WHITE) ch = (char)std::toupper((unsigned char)ch);
    return ch;
}

static inline bool char_to_piece(char ch, Color &c, PieceType &pt) {
    if (ch >= 'A' && ch <= 'Z') c = WHITE;
    else if (ch >= 'a' && ch <= 'z') c = BLACK;
    else return false;

    char lo = (char)std::tolower((unsigned char)ch);
    switch (lo) {
        case 'p': pt = PAWN;   return true;
        case 'n': pt = KNIGHT; return true;
        case 'b': pt = BISHOP; return true;
        case 'r': pt = ROOK;   return true;
        case 'q': pt = QUEEN;  return true;
        case 'k': pt = KING;   return true;
        default: return false;
    }
}

bool Position::set_fen(const std::string& fen) {
    clear();

    std::istringstream iss(fen);
    std::string board, side, castle, ep;
    int half = 0, full = 1;

    if (!(iss >> board >> side >> castle >> ep)) return false;
    // half/full are optional in some inputs
    if (iss >> half >> full) {
        halfmove_clock = half;
        fullmove_number = full;
    } else {
        halfmove_clock = 0;
        fullmove_number = 1;
    }

    // 1) board
    int r = 7;
    int f = 0;

    for (char ch : board) {
        if (ch == '/') {
            r--;
            f = 0;
            if (r < 0) return false;
            continue;
        }

        if (std::isdigit((unsigned char)ch)) {
            f += ch - '0';
            if (f > 8) return false;
            continue;
        }

        Color c;
        PieceType pt;
        if (!char_to_piece(ch, c, pt)) return false;

        if (f < 0 || f > 7 || r < 0 || r > 7) return false;
        Square sq = mk_sq(f, r);
        pieces[c][pt] |= bb_of(sq);
        f++;
    }

    if (r != 0) {
        // if we didn't end on rank 1 after parsing, it's malformed
        // (some malformed FENs might still pass; we’ll be strict)
        // But allow exact finish at r==0 and f==8.
    }

    // 2) side to move
    if (side == "w") stm = WHITE;
    else if (side == "b") stm = BLACK;
    else return false;

    // 3) castling
    castling_rights = 0;
    if (castle != "-") {
        for (char cch : castle) {
            switch (cch) {
                case 'K': castling_rights |= uint8_t(WHITE_KING_SIDE);  break;
                case 'Q': castling_rights |= uint8_t(WHITE_QUEEN_SIDE); break;
                case 'k': castling_rights |= uint8_t(BLACK_KING_SIDE);  break;
                case 'q': castling_rights |= uint8_t(BLACK_QUEEN_SIDE); break;
                default: return false;
            }
        }
    }

    // 4) en passant
    en_passant_square = NO_SQUARE;
    if (ep != "-") {
        if (ep.size() != 2) return false;
        char file = ep[0];
        char rank = ep[1];
        if (file < 'a' || file > 'h') return false;
        if (rank < '1' || rank > '8') return false;
        int ff = file - 'a';
        int rr = rank - '1';
        en_passant_square = mk_sq(ff, rr);
    }

    update_occ();

    // sanity: exactly one king each
    if (popcount(pieces[WHITE][KING]) != 1) return false;
    if (popcount(pieces[BLACK][KING]) != 1) return false;

    return true;
}

std::string Position::fen() const {
    // 1) board
    std::string out;
    out.reserve(100);

    for (int r = 7; r >= 0; --r) {
        int emptyCount = 0;
        for (int f = 0; f < 8; ++f) {
            Square sq = mk_sq(f, r);
            Bitboard m = bb_of(sq);

            char ch = 0;
            for (int c = 0; c < 2 && !ch; ++c) {
                for (int p = 0; p < 6; ++p) {
                    if (pieces[c][p] & m) {
                        ch = piece_to_char((Color)c, (PieceType)p);
                        break;
                    }
                }
            }

            if (!ch) {
                emptyCount++;
            } else {
                if (emptyCount) {
                    out.push_back(char('0' + emptyCount));
                    emptyCount = 0;
                }
                out.push_back(ch);
            }
        }
        if (emptyCount) out.push_back(char('0' + emptyCount));
        if (r) out.push_back('/');
    }

    // 2) stm
    out.push_back(' ');
    out.push_back(stm == WHITE ? 'w' : 'b');

    // 3) castling
    out.push_back(' ');
    std::string cast;
    if (castling_rights & WHITE_KING_SIDE)  cast.push_back('K');
    if (castling_rights & WHITE_QUEEN_SIDE) cast.push_back('Q');
    if (castling_rights & BLACK_KING_SIDE)  cast.push_back('k');
    if (castling_rights & BLACK_QUEEN_SIDE) cast.push_back('q');
    if (cast.empty()) cast = "-";
    out += cast;

    // 4) ep
    out.push_back(' ');
    if (en_passant_square == NO_SQUARE) {
        out.push_back('-');
    } else {
        int ff = f_of(en_passant_square);
        int rr = r_of(en_passant_square);
        out.push_back(char('a' + ff));
        out.push_back(char('1' + rr));
    }

    // 5) clocks
    out.push_back(' ');
    out += std::to_string(halfmove_clock);
    out.push_back(' ');
    out += std::to_string(fullmove_number);

    return out;
}

} // namespace chess
