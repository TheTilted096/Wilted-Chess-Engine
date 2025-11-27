// Function Definitions for History

#include "History.h"

History::History(){
    empty();
}

void History::empty(){
    std::memset(&quiet, 0, sizeof(quiet));
    std::memset(&noisy, 0, sizeof(noisy));
}

void History::updateQuiet(const Move& m, const Color& c, int t){
    Square sq = m.to();
    Piece p = m.moving();

    int16_t s = static_cast<int16_t>(std::clamp(t, static_cast<int>(-QUIET_HISTORY_LIM), 
        static_cast<int>(QUIET_HISTORY_LIM)));

    quiet[c][p][sq] += (s - quiet[c][p][sq] * s / QUIET_HISTORY_LIM);
}

void History::updateNoisy(const Move& m, const Color& c, int t){
    int16_t s = static_cast<int16_t>(std::clamp(t, static_cast<int>(-QUIET_HISTORY_LIM), 
        static_cast<int>(QUIET_HISTORY_LIM)));

    noisy[c][m.moving()][m.captured()][m.to()] += 
        (s - noisy[c][m.moving()][m.captured()][m.to()] * s / QUIET_HISTORY_LIM);
}

int16_t History::quietEntry(const Move& m, const Color& c){
    return quiet[c][m.moving()][m.to()];
}

int16_t History::noisyEntry(const Move&m, const Color& c){
    return noisy[c][m.moving()][m.captured()][m.to()];
}