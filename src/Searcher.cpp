// Function Definitions for Searcher

#include "Searcher.h"

Searcher::Searcher() : pos(), eva(), tim() {
    nodes = 0ULL;
    hardNodeMax = ~0ULL;
    lifeNodes = 0ULL;

    bestMove = Move::Invalid;

    gen.assign(&pos);
    eva.assign(&pos);

    newGame();

    //movesDeep = 0;
}

bool Searcher::invokeMove(const Move& m){
    pos.makeMove(m);

    if (gen.illegal()){
        pos.unmakeMove();
        return false;
    }

    eva.doMove(m);
    nodes++;

    return true;
}

void Searcher::revokeMove(const Move& m){
    pos.unmakeMove();
    eva.undoMove(m);
}

void Searcher::scoreMoves(MoveList& ml, std::array<uint32_t, MOVELIST_SIZE>& points, const Index& len){
    for (Index i = 0; i < len; i++){
        if (ml[i].captured()){ //capture
            points[i] = (1U << 26) + ml[i].moving() - (ml[i].captured() << 16); //MVV LVA
            continue;
        }

        points[i] = ml[i].moving(); //LVA
    }
}

void Searcher::scoreCaptures(MoveList& ml, std::array<uint32_t, MOVELIST_SIZE>& points, const Index& len){
    for (Index i = 0; i < len; i++){
        points[i] = (1U << 26) + ml[i].moving() - (ml[i].captured() << 16);
    }
}

void Searcher::sortMoves(MoveList& ml, std::array<uint32_t, MOVELIST_SIZE>& points, const Index& len){
    uint32_t keyPoints;
    Move keyMove;

    Index j;
    for (Index i = 1; i < len; i++){
        keyPoints = points[i];
        keyMove = ml[i];
        j = i;
        while ((j > 0) and (points[j - 1] < keyPoints)){
            points[j] = points[j - 1];
            ml[j] = ml[j - 1];
            j--;
        }

        points[j] = keyPoints;
        ml[j] = keyMove;
    }
}

void Searcher::newGame(){
    pos.setStartPos();
    pvt.clearAll();
}

void Searcher::maybeForceStop(){
    if (tim.timeKept and (nodes % 2048 == 0)){
        if (tim.exceedHard()){
            throw "Hard Time Limit\n";
        }
    }

    if (nodes > hardNodeMax){
        throw "Nodes Exceeded\n";
    }
}

Score Searcher::quiesce(Score alpha, Score beta){
    //insufficient material
    if (pos.insufficient()){
        return DRAW;
    }

    Score score = eva.judge();
    Score bestScore = score;

    if (score >= beta){
        return score;
    }
    if (score > alpha){
        alpha = score;
    }

    MoveList captures;
    Count captureCount = gen.generateCaptures(captures);

    std::array<uint32_t, MOVELIST_SIZE> capturePower;
    scoreCaptures(captures, capturePower, captureCount);
    sortMoves(captures, capturePower, captureCount);

    for (Index i = 0; i < captureCount; i++){

        maybeForceStop();

        bool legal = invokeMove(captures[i]);
        if (!legal){ continue; }

        score = -quiesce(-beta, -alpha);

        revokeMove(captures[i]);

        if (score >= beta){
            return score;
        }
        if (score > alpha){
            alpha = score;
        }
        if (score > bestScore){
            bestScore = score;
        }
    }

    return bestScore;
}

template <bool isPV> Score Searcher::alphabeta(Score alpha, Score beta, Depth depth, Index ply){
    Score score = DEFEAT;
    Score bestScore = DEFEAT;

    bool rootNode = (ply == 0);

    pvt.clearLine(ply);

    Count repeats = pos.repetitions(0);
    if (repeats > 2){
        return DRAW;
    }

    //50 move rule
    if (pos.sinceReset() >= 100){
        return DRAW;
    }

    // Insufficient Mating Material
    if (pos.insufficient()){
        return DRAW;
    }

    if (depth == 0){
        return quiesce(alpha, beta);
    }

    if constexpr (!isPV){ //mate distance pruning
        Score ma = std::max(alpha, static_cast<Score>(DEFEAT + ply));
        Score mb = std::min(beta, static_cast<Score>(VICTORY - ply - 1));
        if (ma >= mb){
            return ma;
        }
    }

    bool inCheck = gen.isChecked(pos.toMove); //maybe clean up these functions

    MoveList moves;
    Count moveCount = gen.generateMoves(moves);

    std::array<uint32_t, MOVELIST_SIZE> movePower;
    scoreMoves(moves, movePower, moveCount);
    sortMoves(moves, movePower, moveCount);
    
    Count numLegal = 0;

    for (Index i = 0; i < moveCount; i++){

        maybeForceStop();

        bool legal = invokeMove(moves[i]);
        if (!legal){ continue; }

        numLegal++;

        if (numLegal == 1){
            score = -alphabeta<true>(-beta, -alpha, depth - 1, ply + 1);
        } else {
            score = -alphabeta<false>(-alpha - 1, -alpha, depth - 1, ply + 1);

            if ((score > alpha) and isPV){
                score = -alphabeta<true>(-beta, -alpha, depth - 1, ply + 1);
            }
        }
        
        
        revokeMove(moves[i]);

        if (score > bestScore){
            bestScore = score;
        }

        if (score > alpha){ // PV Node
            alpha = score;

            if (rootNode){
                bestMove = moves[i];
            }

            if constexpr (isPV){
                pvt.write(moves[i], ply);
            }
        }

        if (score >= beta){ //Cut Node
            return score;
        }

    }

    if (bestScore == DEFEAT){
        if (inCheck){ return ply + DEFEAT; } // mate in ply
        return DRAW; //if not in check but we had no moves, its a stalemate
    }

    return bestScore;
}

template Score Searcher::alphabeta<true>(Score, Score, Depth, Index);
template Score Searcher::alphabeta<false>(Score, Score, Depth, Index);

Score Searcher::search(Depth depthLim, uint64_t nodeLim, bool output){
    eva.refresh();
    nodes = 0;

    std::array<Bitboard, 2> sidesi = pos.sides;
    std::array<Bitboard, 6> piecesi = pos.pieces;
    Color toMovei = pos.toMove;
    Index clocki = pos.clock;

    tim.start();
    hardNodeMax = nodeLim;

    Score searchScore;
    Move currentBest(Move::Invalid);

    depthLim = std::min(MAX_PLY, depthLim);

    int64_t dur;
    int64_t nps;

    try {
        for (Depth d = 1; d <= depthLim; d++){
            searchScore = alphabeta<true>(-SCORE_INF, SCORE_INF, d, 0);
            
            currentBest = bestMove;
            if (output){
                std::cout << "info depth " << (int)d << " score cp " << searchScore 
                    << " nodes " << nodes;

                dur = tim.elapsed();
                nps = 1000000 * nodes / dur;

                std::cout << " nps " << nps << " time " << (dur / 1000);
                std::cout << " pv ";
                for (int i = 0; i < pvt.heights[0]; i++){
                    std::cout << pos.moveName(pvt.vars[i]) << ' ';
                }
                std::cout << std::endl;
            }

            if (tim.timeKept and tim.exceedSoft()){
                //std::cout << "Soft\n";
                break;
            }
        }
    } catch (const char* e){
        //std::cout << e << '\n';
        bestMove = currentBest;
    }

    if (output){
        dur = tim.elapsed();
        nps = 1000000 * nodes / dur;

        std::cout << "info nodes " << nodes << " nps " << nps << std::endl;
        std::cout << "bestmove " << pos.moveName(bestMove) << std::endl;
    }

    pos.sides = sidesi;
    pos.pieces = piecesi;
    pos.toMove = toMovei;
    pos.clock = clocki;

    lifeNodes += nodes;

    return searchScore;
}

void Searcher::bench(){
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

    auto benchStart = std::chrono::steady_clock::now();

    for (std::string tester : marks){
        newGame();
        pos.readFen(tester);
        search(7, ~0ULL, false);
    }

    auto benchEnd = std::chrono::steady_clock::now();
    int64_t dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(benchEnd - benchStart).count();
    int64_t nps = 1000000 * lifeNodes / dur;

    std::cout << lifeNodes << " nodes " << nps << " nps " << std::endl;
}






