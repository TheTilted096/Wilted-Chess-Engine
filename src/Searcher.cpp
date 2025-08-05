// Function Definitions for Searcher

#include "Searcher.h"

template <bool isMaster> 
Searcher<isMaster>::Searcher(){
    hardNodeMax = ~0ULL;

    bestMove = Move::Invalid;

    gen.assign(&pos);
    eva.assign(&pos);

    pvt = nullptr;
    tim = nullptr;
    stopSearch = nullptr;

    nodesptr = nullptr;
    nodesArrPtr = nullptr;

    for (Depth i = 0; i < MAX_PLY; i++){
        for (Index j = 0; j < 128; j++){
            LMRtable[i][j] = LMRbase + LMRmult * log(i + 1) * log(j + 1);
        }
    }

    newGame();
}

template <bool isMaster> 
void Searcher<isMaster>::assign(std::atomic<bool>* s, TeaTable* ts, std::atomic<uint64_t>* np){
    stopSearch = s;
    ttref = ts;
    nodesptr = np;
}

template <bool isMaster>
void Searcher<isMaster>::promote(Princes* p, Timeman* t, SharedArray<uint64_t>* sa){
    pvt = p;
    tim = t;
    nodesArrPtr = sa;
}

template <bool isMaster> 
bool Searcher<isMaster>::invokeMove(const Move& m){
    pos.makeMove(m);

    if (pos.illegal()){
        pos.unmakeMove();
        return false;
    }

    eva.doMove(m);
    addNode();

    return true;
}

template <bool isMaster> 
void Searcher<isMaster>::revokeMove(const Move& m){
    pos.unmakeMove();
    eva.undoMove(m);
}

template <bool isMaster> 
void Searcher<isMaster>::scoreMoves(MoveList& ml, MoveScoreList& points, const Index& len, const Move& ttm, const Index& p){
    for (Index i = 0; i < len; i++){
        if (ml[i] == ttm){
            points[i] = (1U << 30);
            continue;
        }

        if (ml[i].captured()){ //capture
            points[i] = (1U << 26) + ml[i].moving() - (ml[i].captured() << 16); //MVV LVA
            continue;
        }

        if (ml[i] == sta[p].killer){
            points[i] = (1U << 24);
            continue;
        }

        //points[i] = ml[i].moving(); //LVA
        points[i] = his.quietEntry(ml[i], pos.toMove);
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
void Searcher<isMaster>::clearStack(){
    sta[0].nmp = false;
    sta[0].killer = Move::Null;
}

template <bool isMaster> 
void Searcher<isMaster>::newGame(){
    //pos.setStartPos();
    //his.empty();
    clearStack();
}

template <bool isMaster> 
void Searcher<isMaster>::maybeForceStop(){
    if constexpr (isMaster){ //master handles actual stop conditions
        if (tim->timeKept and (nodes() % 2048 == 0)){
            if (tim->exceedHard()){
                disable();
            }
        }

        if (nodes() > hardNodeMax){
            disable();
        }
    }

    if (stopped()){
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
    if constexpr (isMaster){ //master reports pv
        pvt->clearLine(ply);
    }

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

    sta[ply].presentEval = eva.judge(); // static eval

    if (!rootNode){
        Score margin = RFPbase + RFPmult * depth; //Reverse Futility Pruning
        if ((depth < maxRFPdepth) and !inCheck and !isPV
                and (sta[ply].presentEval - beta > margin)){

            return (sta[ply].presentEval + beta) / 2;
        }

        // Null Move Pruning
        sta[ply].nmp = !sta[ply - 1].nmp and (depth > minNMPdepth)
                and !isPV and !inCheck and !pos.onlyPawns();

        if (sta[ply].nmp){
            pos.passMove();
            Score nullScore = -alphabeta<false>(-beta, -beta + 1, depth - NMPreduce, ply + 1);
            pos.unpassMove();

            if (nullScore >= beta){
                return nullScore;
            }
        }
    }

    MoveList moves;
    Count moveCount = gen.generateMoves(moves);

    MoveScoreList movePower;
    scoreMoves(moves, movePower, moveCount, ttMove, ply);
    sortMoves(moves, movePower, moveCount);
    
    Count numLegal = 0;

    NodeType nodeflag = NodeType::All; //assume fail low unless otherwise
    Move localBestMove = Move::Invalid;

    bool noisy;
    sta[ply + 1].killer = Move::Invalid;

    for (Index i = 0; i < moveCount; i++){

        maybeForceStop();

        bool legal = invokeMove(moves[i]);
        if (!legal){ continue; }

        numLegal++;

        noisy = moves[i].captured();

        if (numLegal == 1){
            score = -alphabeta<isPV>(-beta, -alpha, depth - 1, ply + 1);
        } else {
            Depth r = 0;
            if ((depth > minLMRdepth) and !noisy){
                r = LMRtable[depth][numLegal];
            }

            // search at reduced depth and null window
            // cast to int and max(0) to avoid underflows.
            score = -alphabeta<false>(-alpha - 1, -alpha, std::max(0, static_cast<int>(depth) - 1 - r), ply + 1);

            // if the move does better than expected and we actually reduced the move
            // search null window full depth
            if ((score > alpha) and (r > 0)){
                score = -alphabeta<false>(-alpha - 1, -alpha, depth - 1, ply + 1);
            }

            // the move passes null window search at full depth
            // search the move at full depth and window. isPV must be true to have an actual full window.
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

            if (!noisy){
                his.updateQuiet(moves[i], pos.toMove, static_cast<int16_t>(depth) * static_cast<int16_t>(depth)); //depth squared

                sta[ply].killer = moves[i];
            }

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
    his.empty();
    clearStack();

    hardNodeMax = nodeLim;

    Score searchScore;
    Move currentBest(Move::Invalid);

    int64_t dur;
    int64_t nps;

    Score alpha = -SCORE_INF;
    Score beta = SCORE_INF;
    bool aspFail;
    int prevScore;

    try {
        prevScore = alphabeta<true>(alpha, beta, 0, 0); // depth 0 to get an initial assessment
        // this could be depth 1, but I did not want to copy the rest of the loop body

        for (Depth d = 1; d <= depthLim; d++){
            aspFail = true; 
            int aspAlpha = ASPbase, aspBeta = ASPbase;

            while (aspFail){ // cast to integer to be sure of no overflow
                alpha = std::max(prevScore - aspAlpha, (int)(-SCORE_INF));
                beta = std::min(prevScore + aspBeta, (int)(SCORE_INF));

                searchScore = alphabeta<true>(alpha, beta, d, 0);

                if (searchScore < alpha){ //failed low, must expand window down
                    aspAlpha *= ASPmult;
                    continue;
                }
                if (searchScore > beta){ //failed high, must expand window high
                    aspBeta *= ASPmult;
                    continue;
                }

                aspFail = false; //otherwise, it passed
            }

            //searchScore = alphabeta<true>(-SCORE_INF, SCORE_INF, d, 0);
            
            currentBest = bestMove;
            prevScore = searchScore;

            if constexpr (isMaster and output){
                if (!minPrint){
                    uint64_t pn = pooledNodes();

                    std::cout << "info depth " << (int)d << " score cp " << searchScore 
                        << " nodes " << pn;

                    dur = tim->elapsed();
                    nps = 1000000 * pn / dur;

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
        uint64_t pn = pooledNodes();
        dur = tim->elapsed();
        nps = 1000000 * pn / dur;

        std::cout << "info nodes " << pn << " nps " << nps << std::endl;
        //std::cout << "bestmove " << pos.moveName(bestMove) << std::endl;
    }

    return searchScore;
}

template Score Searcher<true>::search<true>(Depth, uint64_t, bool);
template Score Searcher<true>::search<false>(Depth, uint64_t, bool);
template Score Searcher<false>::search<true>(Depth, uint64_t, bool);
template Score Searcher<false>::search<false>(Depth, uint64_t, bool);

template class Searcher<true>;
template class Searcher<false>;


