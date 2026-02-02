// Function Definitions for History

#include "History.h"

History::History(){
    empty();
}

void History::empty(){
    std::memset(&quiet, 0, sizeof(quiet));
    std::memset(&counters, 0, sizeof(counters));
}

void History::gravity(int16_t& e, int t){
    int16_t s = static_cast<int16_t>(std::clamp(t, static_cast<int>(-QUIET_HISTORY_LIM), 
        static_cast<int>(QUIET_HISTORY_LIM)));

    e += (s - e * std::abs(s) / QUIET_HISTORY_LIM);    
}

void History::updateQuiet(const Move& m, const Color& c, int t){ gravity(quietEntry(m, c), t); }

int16_t& History::quietEntry(const Move& m, const Color& c){ return quiet[c][m.from()][m.to()]; }

void History::updateCounter(const Color& c, const Move& p, const Move& m, int t){
    gravity(counterEntry(c, p, m), t);
}

int16_t& History::counterEntry(const Color& c, const Move& p, const Move& m){
    return counters[c][p.moving()][p.to()][m.moving()][m.to()];
}