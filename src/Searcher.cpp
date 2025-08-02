// Function Definitions for Searcher

#include "Searcher.h"

template <bool isMaster> 
Searcher<isMaster>::Searcher() : pos(), gen(), eva() {
    nodes = 0ULL;
    hardNodeMax = ~0ULL;

    bestMove = Move::Invalid;

    gen.assign(&pos);
    eva.assign(&pos);

    pvt = nullptr;
    tim = nullptr;
    stopSearch = nullptr;

    newGame();
}

template <bool isMaster> 
void Searcher<isMaster>::assign(Princes* p, Timeman* t, bool* s, TeaTable* ts){
    pvt = p;
    tim = t;
    stopSearch = s;
    ttref = ts;
}

template <bool isMaster> 
bool Searcher<isMaster>::invokeMove(const Move& m){
    pos.makeMove(m);

    if (pos.illegal()){
        pos.unmakeMove();
        return false;
    }

    eva.doMove(m);
    nodes++;

    return true;
}

template <bool isMaster> 
void Searcher<isMaster>::revokeMove(const Move& m){
    pos.unmakeMove();
    eva.undoMove(m);
}

template <bool isMaster> 
void Searcher<isMaster>::scoreMoves(MoveList& ml, MoveScoreList& points, const Index& len, const Move& ttm){
    for (Index i = 0; i < len; i++){
        if (ml[i] == ttm){
            points[i] = (1U << 30);
            continue;
        }

        if (ml[i].captured()){ //capture
            points[i] = (1U << 26) + ml[i].moving() - (ml[i].captured() << 16); //MVV LVA
            continue;
        }

        points[i] = ml[i].moving(); //LVA
    }
}

template <bool isMaster>
void Searcher<isMaster>::scoreCaptures(MoveList& ml, MoveScoreList& points, const Index& len){
    for (Index i = 0; i < len; i++){
        points[i] = (1U << 26) + ml[i].moving() - (ml[i].captured() << 16);
    }
}

template <bool isMaster> 
void Searcher<isMaster>::sortMoves(MoveList& ml, MoveScoreList& points, const Index& len){
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

template <bool isMaster> 
void Searcher<isMaster>::newGame(){
    pos.setStartPos();
}

template <bool isMaster> 
void Searcher<isMaster>::maybeForceStop(){
    if constexpr (isMaster){ //master handles actual stop conditions
        if (tim->timeKept and (nodes % 2048 == 0)){
            if (tim->exceedHard()){
                disable();
            }
        }

        if (nodes > hardNodeMax){
            disable();
        }
    }

    if (*stopSearch){
        throw "stopSearch raised";
    }
}

template <bool isMaster> 
Score Searcher<isMaster>::quiesce(Score alpha, Score beta){
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

    MoveScoreList capturePower;
    scoreCaptures(captures, capturePower, captureCount);
    sortMoves(captures, capturePower, captureCount);

    for (Index i = 0; i < captureCount; i++){

        maybeForceStop(); //check if the thread should stop

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

template <bool isMaster>
template <bool isPV> 
Score Searcher<isMaster>::alphabeta(Score alpha, Score beta, Depth depth, Index ply){
    Score score = DEFEAT;
    Score bestScore = DEFEAT;

    bool rootNode = (ply == 0);

    if constexpr (isMaster){ //master reports pv
        pvt->clearLine(ply);
    }

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

    Teacup& probedEntry = ttref->probe(pos.thisHash());
    Move ttMove = Move::Invalid; //perhaps init with Move::Invalid

    if (probedEntry.eHash() == pos.thisHash()){
        score = probedEntry.eScore(ply);
        ttMove = probedEntry.eMove();

        NodeType nt = probedEntry.enType();
        if ((probedEntry.eDepth() >= depth) and (repeats == 1) 
                and !isPV and !rootNode){
            if (nt == NodeType::Exact){ return score; }

            if ((nt == NodeType::Cut) and (score >= beta)){ return score; }

            if ((nt == NodeType::All) and (score <= alpha)){ return score; }
        }
    }

    bool inCheck = pos.isChecked(pos.toMove); //maybe clean up these functions

    MoveList moves;
    Count moveCount = gen.generateMoves(moves);

    MoveScoreList movePower;
    scoreMoves(moves, movePower, moveCount, ttMove);
    sortMoves(moves, movePower, moveCount);
    
    Count numLegal = 0;

    NodeType nodeflag = NodeType::All; //assume fail low unless otherwise
    Move localBestMove = Move::Invalid;

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

            nodeflag = NodeType::Exact;

            localBestMove = moves[i];

            if (rootNode){ // other threads can still have bestmove
                bestMove = moves[i];
            }

            if constexpr (isMaster and isPV){ // pvtable is cosmetic and only applies to master thread
                pvt->write(moves[i], ply);
            }
        }

        if (score >= beta){ //Cut Node
            probedEntry.update(score, NodeType::Cut, depth, pos.thisHash(), moves[i], ply); //update TT in cut node

            return score;
        }

    }

    if (bestScore == DEFEAT){
        if (inCheck){ return ply + DEFEAT; } // mate in ply
        return DRAW; //if not in check but we had no moves, its a stalemate
    }

    probedEntry.update(bestScore, nodeflag, depth, pos.thisHash(), localBestMove, ply);

    return bestScore;
}

/*
template Score Searcher<true>::alphabeta<true>(Score, Score, Depth, Index);
template Score Searcher<true>::alphabeta<false>(Score, Score, Depth, Index);

template Score Searcher<false>::alphabeta<true>(Score, Score, Depth, Index);
template Score Searcher<false>::alphabeta<false>(Score, Score, Depth, Index);
*/

template <bool isMaster>
template <bool output>
Score Searcher<isMaster>::search(Depth depthLim, uint64_t nodeLim, bool minPrint){
    eva.refresh();
    nodes = 0;

    std::array<Bitboard, 2> sidesi = pos.sides;
    std::array<Bitboard, 6> piecesi = pos.pieces;
    Color toMovei = pos.toMove;
    Index clocki = pos.clock;

    hardNodeMax = nodeLim;

    Score searchScore;
    Move currentBest(Move::Invalid);

    int64_t dur;
    int64_t nps;

    try {
        for (Depth d = 1; d <= depthLim; d++){
            searchScore = alphabeta<true>(-SCORE_INF, SCORE_INF, d, 0);
            
            currentBest = bestMove;

            if constexpr (isMaster and output){
                if (!minPrint){
                    std::cout << "info depth " << (int)d << " score cp " << searchScore 
                        << " nodes " << nodes;

                    dur = tim->elapsed();
                    nps = 1000000 * nodes / dur;

                    std::cout << " nps " << nps << " time " << (dur / 1000);
                    std::cout << " pv ";
                    for (int i = 0; i < pvt->heights[0]; i++){
                        std::cout << pos.moveName(pvt->vars[i], i & 1) << ' ';
                    }
                    std::cout << std::endl;
                }
            }

            if constexpr (isMaster){ //only master should be checking time
                if (tim->timeKept and tim->exceedSoft()){
                    disable();
                    break;
                }
            }
        }
    } catch (const char* e){
        bestMove = currentBest;
    }

    if constexpr (isMaster){
        disable(); //search has stopped, regardless.
    }

    if constexpr (isMaster and output){
        dur = tim->elapsed();
        nps = 1000000 * nodes / dur;

        std::cout << "info nodes " << nodes << " nps " << nps << std::endl;
        std::cout << "bestmove " << pos.moveName(bestMove) << std::endl;
    }

    pos.sides = sidesi;
    pos.pieces = piecesi;
    pos.toMove = toMovei;
    pos.clock = clocki;

    return searchScore;
}

template Score Searcher<true>::search<true>(Depth, uint64_t, bool);
template Score Searcher<true>::search<false>(Depth, uint64_t, bool);
template Score Searcher<false>::search<true>(Depth, uint64_t, bool);
template Score Searcher<false>::search<false>(Depth, uint64_t, bool);

template class Searcher<true>;
template class Searcher<false>;


