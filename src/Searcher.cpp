// Function Definitions for Searcher

#include "Searcher.h"

Searcher::Searcher() : pos() {
    nodes = 0ULL;

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

void Searcher::newGame(){
    pos.setStartPos();
}

Score Searcher::alphabeta(Score alpha, Score beta, Depth depth, Index ply){
    Score score = DEFEAT;
    Score bestScore = DEFEAT;

    bool rootNode = (ply == 0);

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
        return eva.judge();
    }

    bool inCheck = gen.isChecked(pos.toMove); //maybe clean up these functions

    Actions moves{};
    Count moveCount = gen.generateMoves(moves);

    for (Index i = 0; i < moveCount; i++){
        bool legal = invokeMove(moves[i]);
        if (!legal){ continue; }

        score = -alphabeta(-beta, -alpha, depth - 1, ply + 1);

        revokeMove(moves[i]);

        if (score > bestScore){
            bestScore = score;
        }

        if (score > alpha){ // PV Node
            alpha = score;

            if (rootNode){
                bestMove = moves[i];
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

Score Searcher::search(){
    eva.refresh();
    nodes = 0;
    moment = std::chrono::steady_clock::now();

    Score result = alphabeta(-SCORE_INF, SCORE_INF, 4, 0);

    auto end = std::chrono::steady_clock::now();
    double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - moment).count();
    int nps = (nodes / dur) * 1000000;
    int time = dur / 1000; //in ms

    std::cout << "info depth 4 score cp " << result << " nodes " << nodes;
    std::cout << " nps " << nps << " time " << time << '\n';

    std::cout << "bestmove " << pos.moveName(bestMove) << std::endl;

    return result;
}