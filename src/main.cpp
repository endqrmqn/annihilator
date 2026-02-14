#include "uci/uci.hpp"
#include "chess/attacks.hpp"

int main() {
    chess::init_attack_tables(); // MUST run once before movegen
    uci::loop();
    return 0;
}
