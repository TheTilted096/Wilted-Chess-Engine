// Function Definitions for Engine

#include "Engine.h"

Engine::Engine() : master(), pvtable(), timer(), stopFlag(false){
    master.assign(&pvtable, &timer, &stopFlag, &ttable);
}

void Engine::newGame(){
    ttable.clear();
    pvtable.clearAll();
    master.newGame();
}

void Engine::bench(){
    std::string marks[8] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r1bqkb1r/2p2ppp/p1np1n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 b kq - 1 7",
        "3r2k1/2pr2p1/p2bpn1p/Pp2p3/1Pq1P3/2PR1N1P/2Q2PP1/2B1R1K1 w - - 5 28",
        "2r1qrk1/1b1n2p1/1p1ppn1p/p1p2p2/P1PP4/2PBPP2/2Q2NPP/2B1RRK1 b - - 1 10",
        "6k1/5pp1/4n3/3p4/r6Q/6P1/q4PBP/3R3K w - - 0 35",
        "6k1/6p1/5p1p/pR1b4/P1r5/4PN2/2P3PP/6K1 b - - 2 35",
        "8/8/5k2/8/8/6P1/6K1/8 w - - 0 57",
        "6k1/8/6p1/8/8/6P1/4qQKP/8 b - - 5 49"
    };

    //auto benchStart = std::chrono::steady_clock::now();

    uint64_t lifeNodes = 0ULL;

    timer.start();

    for (std::string tester : marks){
        newGame();
        stopFlag = false;
        
        master.pos.readFen(tester);
        master.search<false>(14, ~0ULL, false);
        lifeNodes += master.nodes;
    }

    //auto benchEnd = std::chrono::steady_clock::now();
    //int64_t dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(benchEnd - benchStart).count();
    int64_t dur = timer.elapsed();
    int64_t nps = 1000000 * lifeNodes / dur;

    std::cout << lifeNodes << " nodes " << nps << " nps " << std::endl;
}

template <bool out> Score Engine::go(Depth d, uint64_t nl, bool mp){
    stopFlag = false;
    timer.start();

    d = std::min(MAX_PLY, d);

    Score result = master.search<out>(d, nl, mp);

    return result;
}

template Score Engine::go<true>(Depth, uint64_t, bool);
template Score Engine::go<false>(Depth, uint64_t, bool);
