// Function Definitions for History

#include "History.h"

History::History(){
    empty();
}

void History::empty(){
    std::memset(&quiet, 0, sizeof(quiet));
}

void History::updateQuiet(const Move& m, const Color& c, int t){
    int16_t s = static_cast<int16_t>(std::clamp(t, static_cast<int>(-QUIET_HISTORY_LIM), 
        static_cast<int>(QUIET_HISTORY_LIM)));

    quiet[c][m.from()][m.to()] += (s - quiet[c][m.from()][m.to()] * std::abs(s) / QUIET_HISTORY_LIM);
}

int16_t History::quietEntry(const Move& m, const Color& c){
    return quiet[c][m.from()][m.to()];
}