/*
Main Driver Code for Wilted Engine

TheTilted096
12-19-24

*/

#include "WiltedPosition.h"

int main(int argc, char* argv[]){
    Bitboards::initSlideAttacks();

    Position p;

    p.setStartPos();
    //p.readFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"); //Rook Endgame
    //p.readFen("4k3/8/8/8/8/8/P6P/R3K2R w KQ - 0 1"); //castling test
    //p.readFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
    //p.print();

    int mp = 7;
    
    uint32_t ml[] = {1477168, 1498543, 1496347, 700406, 0, 0, 0, 0}; 
    //a2a3 h3g2 d5e6 g2h1r
    int v = 0;
    Move mt;
    
    for (int i = 0; i < v; i++){
        mt = Move(ml[i]);
        p.makeRMove(Move(ml[i]));
    }

    //p.print();

    auto start = std::chrono::steady_clock::now();

    uint64_t pn = p.perft(mp - v, 0);

    auto end = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    int nps = (dur != 0) ? pn / ((double) dur) * 1000000 : 0;
    std::cout << nps << " nps\n\n";

    //p.print();

    return 0;
}