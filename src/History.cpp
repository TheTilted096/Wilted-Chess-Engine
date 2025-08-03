// Function Definitions for History

#include "History.h"

History::History(){
    empty();
}

void History::empty(){
    for (Piece p = King; p < None; p++){
        for (Square sq = a8; sq < XX; sq++){
            quiet[White][p][sq] = 0;
            quiet[Black][p][sq] = 0;
        }
    }
}

void History::updateQuiet(const Move& m, const Color& c, const int16_t& s){
    Square sq = m.to();
    Piece p = m.moving();
    
    quiet[c][p][sq] += (s - quiet[c][p][sq] * s / QUIET_HISTORY_LIM);
}

int16_t History::quietEntry(const Move& m, const Color& c){
    return quiet[c][m.moving()][m.to()];
}