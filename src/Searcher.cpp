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
void Searcher<isMaster>::invokeMove(const Move& m){
    pos.makeMove(m);

    eva.doMove(m);
    addNode();
}

template <bool isMaster> 
void Searcher<isMaster>::revokeMove(const Move& m){
    pos.unmakeMove();
    eva.undoMove();
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

            if (!see(ml[i], 0)){
                points[i] -= (1U << 27);
            }
            
            continue;
        }

        if (ml[i] == sta[p].killer){
            points[i] = (1U << 24);
            continue;
        }

        //points[i] = ml[i].moving(); //LVA
        points[i] = his.quietEntry(ml[i], pos.toMove);
        Move lp = pos.lastPlayed();
        points[i] += !lp.isNull() * his.counterEntry(pos.toMove, lp, ml[i]);
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
    int keyPoints;
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
    //sta[0].nmp = false;
    sta[0].killer = Move::Null;
}

template <bool isMaster> 
void Searcher<isMaster>::newGame(){
    //pos.setStartPos();
    his.empty();
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
bool Searcher<isMaster>::see(const Move& m, const Score& bound){
    Color insideToMove = pos.toMove; // start internal side to move

    std::array<Score, 2> gains{}; // gains for each side. initialize bound below and add capture if any
    gains[insideToMove] = !!m.captured() * SEEvals[m.captured()] - bound; // start with the capture

    if (gains[insideToMove] >= SEEvals[m.moving()]){ // if gain of move is more than loss
        return true; //it is at least an equal trade and so it passes
    }

    Square destination = m.to();
    bool backrank = 0xFF000000000000FFULL & squareBitboard(destination); // for autoqueen purposes

    gains[insideToMove] += m.promoted() * (SEEvals[m.ending()] - SEEvals[Pawn]); // if queening

    // get all attackers

    Bitboard occ = pos.occupied() ^ squareBitboard(m.from()); //remove attacker

    Table<Count, 2, 8> attackers{}; // exact # of attackers for P, N, K, attacker square for B, R, Q
    std::array<Hash, 2> attackerHash{}; // u64 cast of attackers[side]

    // !insideToMove attackers
    attackers[!insideToMove][Pawn] = std::popcount(Attacks::PawnAttacks[insideToMove][destination] & pos.those(flip(insideToMove), Pawn) & occ);

    attackers[!insideToMove][Knight] = std::popcount(Attacks::KnightAttacks[destination] & pos.those(flip(insideToMove), Knight) & occ);

    Bitboard bsb = Attacks::bishopAttacks(destination, occ);
    Bitboard rsb = Attacks::rookAttacks(destination, occ);

    std::array<Bitboard, 2> queenbb{}; // bitboard of all queens

    attackers[!insideToMove][Bishop] = getLeastBit(bsb & pos.those(flip(insideToMove), Bishop) & occ) ^ 64;
    attackers[!insideToMove][Rook] = getLeastBit(rsb & pos.those(flip(insideToMove), Rook) & occ) ^ 64;

    queenbb[!insideToMove] = (bsb ^ rsb) & pos.those(flip(insideToMove), Queen) & occ;
    attackers[!insideToMove][Queen] = getLeastBit(queenbb[!insideToMove]) ^ 64;

    attackers[!insideToMove][King] = !!(Attacks::KingAttacks[destination] & pos.those(flip(insideToMove), King) & occ);

    // no enemy attackers and positive gains (the piece was hanging)
    attackerHash[!insideToMove] = *reinterpret_cast<const uint64_t*>(attackers[!insideToMove].data());
    if (!attackerHash[!insideToMove] and (gains[insideToMove] >= 0)){
        return true;
    }

    // insideToMove attackers
    attackers[insideToMove][Pawn] = std::popcount(Attacks::PawnAttacks[!insideToMove][destination] & pos.those(insideToMove, Pawn) & occ);

    attackers[insideToMove][Knight] = std::popcount(Attacks::KnightAttacks[destination] & pos.those(insideToMove, Knight) & occ);
    
    attackers[insideToMove][Bishop] = getLeastBit(bsb & pos.those(insideToMove, Bishop) & occ) ^ 64;
    attackers[insideToMove][Rook] = getLeastBit(rsb & pos.those(insideToMove, Rook) & occ) ^ 64;

    queenbb[insideToMove] = (bsb ^ rsb) & pos.those(insideToMove, Queen) & occ;
    attackers[insideToMove][Queen] = getLeastBit(queenbb[insideToMove]) ^ 64;

    attackers[insideToMove][King] = !!(Attacks::KingAttacks[destination] & pos.those(insideToMove, King) & occ);

    attackerHash[insideToMove] = *reinterpret_cast<const uint64_t*>(attackers[insideToMove].data());

    Piece standing = m.ending(); // piece standing on square
    insideToMove = flip(insideToMove); // get ready for main loop
    Piece lva;

    Bitboard sqbb;

    while (attackerHash[insideToMove]){ // while stm still has attackers
        gains[insideToMove] += SEEvals[standing]; // piece standing on square is captured
        
        lva = static_cast<Piece>(getMostBit(attackerHash[insideToMove]) >> 3);

        switch (lva){
            case Bishop:
                occ ^= squareBitboard(static_cast<Square>(attackers[insideToMove][Bishop] ^ 64)); // remove backup bit to convert to square
                
                bsb = Attacks::bishopAttacks(destination, occ);

                // if there are none, getLeastBit returns 64, which is then xor'd to 0 
                attackers[insideToMove][Bishop] = 64 ^ getLeastBit(bsb & pos.those(insideToMove, Bishop) & occ);

                queenbb[insideToMove] |= (bsb & pos.those(insideToMove, Queen) & occ);
                attackers[insideToMove][Queen] = 64 ^ getLeastBit(queenbb[insideToMove]);

                break;
            case Rook:
                occ ^= squareBitboard(static_cast<Square>(attackers[insideToMove][Rook] ^ 64)); 

                rsb = Attacks::rookAttacks(destination, occ);

                attackers[insideToMove][Rook] = 64 ^ getLeastBit(rsb & pos.those(insideToMove, Rook) & occ);

                queenbb[insideToMove] |= (rsb & pos.those(insideToMove, Queen) & occ);
                attackers[insideToMove][Queen] = 64 ^ getLeastBit(queenbb[insideToMove]);

                break;
            case Queen:
                sqbb = squareBitboard(static_cast<Square>(attackers[insideToMove][Queen] ^ 64));
                occ ^= sqbb; // remove backup bit to convert to square
                queenbb[insideToMove] ^= sqbb;

                bsb = Attacks::bishopAttacks(destination, occ);
                rsb = Attacks::rookAttacks(destination, occ);

                queenbb[insideToMove] |= ((bsb | rsb) & pos.those(insideToMove, Queen) & occ);
                attackers[insideToMove][Queen] = 64 ^ getLeastBit(queenbb[insideToMove]);

                attackers[insideToMove][Rook] = 64 ^ getLeastBit(rsb & pos.those(insideToMove, Rook) & occ);
                attackers[insideToMove][Bishop] = 64 ^ getLeastBit(bsb & pos.those(insideToMove, Bishop) & occ);

                break;
            default:
                attackers[insideToMove][lva]--;
        }

        bool promote = (lva == 5) and backrank;
        standing = static_cast<Piece>(lva - (promote << 2)); //Pawn - 4 = Queen

        if (gains[insideToMove] - gains[!insideToMove] > SEEvals[standing] * !!attackerHash[!insideToMove]){
            break;
        }

        attackerHash[insideToMove] = *reinterpret_cast<const uint64_t*>(attackers[insideToMove].data());
        insideToMove = flip(insideToMove);
    }

    return (gains[pos.toMove] >= gains[!pos.toMove]);
}

template <bool isMaster> 
Score Searcher<isMaster>::quiesce(Score alpha, Score beta){
    //insufficient material
    if (pos.insufficient()){
        return DRAW;
    }

    Score score = eva.inference();
    //Score score = eva.refresh();
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

        if (!see(captures[i], 0)){ // QS SEE Pruning
            continue;
        }

        invokeMove(captures[i]);

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

    // Mate Distance Pruning
    alpha = std::max(alpha, static_cast<Score>(DEFEAT + ply));
    beta = std::min(beta, static_cast<Score>(VICTORY - ply - 1));
    if (alpha >= beta){
        return alpha;
    }

    if (depth == 0){
        return quiesce(alpha, beta);
    }

    Teacup& probedEntry = ttref->probe(pos.thisHash());
    Move ttMove = Move::Null; //perhaps init with Move::Invalid

    if (probedEntry.eHash() == pos.thisHash() and probedEntry.eGen() == ttref->generation){
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

    bool improving = (ply > 1) and !inCheck and (sta[ply].presentEval > sta[ply - 2].presentEval);

    //std::cout << "static eval for this node:\n";
    sta[ply].presentEval = eva.inference(); // static eval
    //sta[ply].presentEval = eva.refresh();

    if (ttMove == Move::Null and depth > minIIRdepth){
        depth -= IIRbase;
    }

    if (!rootNode){
        Score margin = RFPbase + RFPmult * (depth - improving); //Reverse Futility Pruning
        if ((depth < maxRFPdepth) and !inCheck and !isPV
                and (sta[ply].presentEval - beta > margin)){

            return (sta[ply].presentEval + beta) / 2;
        }

        // Null Move Pruning
        sta[ply].nmp = !sta[ply - 1].nmp and (depth > minNMPdepth)
                and !isPV and !inCheck and !pos.onlyPawns()
                and (sta[ply].presentEval > beta + NMPevalBase - (depth + improving) * NMPevalMult);

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

    int numQuiet = 0;
    MoveList quietSeen{};

    for (Index i = 0; i < moveCount; i++){

        maybeForceStop();

        //bool legal = invokeMove(moves[i]);
        //if (!legal){ continue; }

        //std::cout << "depth: " << (int)depth << " ply:" << (int)ply << " move:" << pos.moveName(moves[i]) << '\n';

        numLegal++;

        noisy = moves[i].captured();

        if (!noisy){
            quietSeen[numQuiet] = moves[i];
            numQuiet++;
        }

        const int16_t moveHist = his.quietEntry(moves[i], pos.toMove);

        if (!isPV and (bestScore > DEFEAT)){ // move loop pruning
            //if (ply == 1){ std::cout << bestScore << '\n';}
            //assert(localBestMove != Move::Invalid);
            if (!noisy){
                // late move pruning
                int lmpCount = (LMPbase + depth * LMPlin + depth * depth * LMPquad);
                if (numQuiet > lmpCount){
                    continue;
                }

                // futility pruning
                if (depth < maxFPdepth and (sta[ply].presentEval + FPbase + FPmult * depth < alpha)){
                    continue;
                }

                // history pruning
                if (depth < histPruneDepth and moveHist < -histPruneFactor * depth){
                    continue;
                }

                // see pruning
                if (depth < maxSEEdepth and !see(moves[i], -static_cast<Score>(depth * SEElin))){
                    continue;
                }
            }
        }

        invokeMove(moves[i]);

        if (numLegal == 1){
            score = -alphabeta<isPV>(-beta, -alpha, depth - 1, ply + 1);
        } else {
            Depth r = 0;
            if ((depth > minLMRdepth) and !noisy){
                r = LMRtable[depth][numLegal];

                r -= isPV;
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

        //std::cout << "score: " << score << '\n';

        if (score > bestScore){
            bestScore = score;
        }

        if (score > alpha){ // PV Node
            alpha = score;

            //std::cout << pos.moveName(moves[i]) << '\n';

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
            probedEntry.update(score, NodeType::Cut, depth, pos.thisHash(), moves[i], ply, ttref->generation); //update TT in cut node

            if (!noisy){
                int bonus = depth * depth * depth; //depth cubed, computed in int to avoid overflow
                his.updateQuiet(moves[i], pos.toMove, bonus);

                Move lp = pos.lastPlayed(); // countermoves
                his.updateCounter(pos.toMove, lp, moves[i], !lp.isNull() * bonus);

                int malus = depth * depth;
                for (int qc = 0; qc < numQuiet - 1; qc++){
                    his.updateQuiet(quietSeen[qc], pos.toMove, -malus);
                    his.updateCounter(pos.toMove, lp, quietSeen[qc], !lp.isNull() * -malus);
                }

                sta[ply].killer = moves[i];
            }

            return score;
        }

    }

    if (bestScore == DEFEAT){
        if (inCheck){ return ply + DEFEAT; } // mate in ply
        return DRAW; //if not in check but we had no moves, its a stalemate
    }

    probedEntry.update(bestScore, nodeflag, depth, pos.thisHash(), localBestMove, ply, ttref->generation);

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
Score Searcher<isMaster>::search(Depth depthLim, uint64_t nodeLim, uint64_t softNodeLim, bool minPrint){
    //std::cout << eva.refresh() << '\n';
    eva.refresh();
    //his.empty();
    clearStack();

    hardNodeMax = nodeLim;

    Score searchScore;
    Move currentBest(Move::Invalid);

    int64_t dur;
    int64_t nps;

    Score alpha = -SCORE_INF;
    Score beta = SCORE_INF;
    bool aspFail;
    int prevScore = 0;

    try {
        prevScore = alphabeta<true>(alpha, beta, 0, 0); // depth 0 to get an initial assessment
        // this could be depth 1, but I did not want to copy the rest of the loop body

        for (Depth d = 1; d <= depthLim; d++){
            aspFail = true; 
            int aspAlpha = ASPbase, aspBeta = ASPbase;
            //std::cout << "eval at depth " << (int)d << ": " << eva.inference() << '\n';

            while (aspFail){ // cast to integer to be sure of no overflow
                alpha = std::max(prevScore - aspAlpha, (int)(-SCORE_INF));
                beta = std::min(prevScore + aspBeta, (int)(SCORE_INF));

                //std::cout << "\n[" << alpha << ", " << beta << "]\n";

                searchScore = alphabeta<true>(alpha, beta, d, 0);

                //std::cout << "searchScore: " << searchScore << '\n';

                if (searchScore <= alpha){ //failed low, must expand window down
                    aspAlpha *= ASPmult;
                    continue;
                }
                if (searchScore >= beta){ //failed high, must expand window high
                    aspBeta *= ASPmult;
                    continue;
                }

                aspFail = false; //otherwise, it passed
                //std::cout << "Aspirations Passed\n";
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

                    /*
                    if (pvt->lens[0] > d){
                        std::cout << "\nPV too long\n";
                        exit(1);
                    }

                    if (pvt->lens[0] == 0){
                        std::cout << "\nzero length pv\n";
                        exit(1);
                    }
                    */

                    for (int i = 0; i < pvt->lens[0]; i++){
                        std::cout << pos.moveName(pvt->vars[0][i], i & 1) << ' ';
                    }
                    std::cout << std::endl;
                }
            }

            if constexpr (isMaster){ //only master should be checking time
                if (tim->timeKept and tim->exceedSoft()){
                    disable();
                    break;
                }

                if (nodes() > softNodeLim){ //soft nodes
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

        std::cout << "info score cp " << prevScore << " nodes " << pn << " nps " << nps << std::endl;
        //std::cout << "bestmove " << pos.moveName(bestMove) << std::endl;
    }

    // Print quiet history values for all quiet moves
    /*
    MoveList allMoves;
    Count moveCount = gen.generateMoves(allMoves);
    std::cout << "\nQuiet History Values:\n";
    for (Index i = 0; i < moveCount; i++){
        if (!allMoves[i].captured()){ // only quiet moves
            int16_t histScore = his.quietEntry(allMoves[i], pos.toMove);
            std::cout << pos.moveName(allMoves[i]) << ": " << histScore << "\n";
        }
    }
    std::cout << std::endl;
    */

    return prevScore; // return score of last complete depth search
}

template Score Searcher<true>::search<true>(Depth, uint64_t, uint64_t, bool);
template Score Searcher<true>::search<false>(Depth, uint64_t, uint64_t, bool);
template Score Searcher<false>::search<true>(Depth, uint64_t, uint64_t, bool);
template Score Searcher<false>::search<false>(Depth, uint64_t, uint64_t, bool);

template class Searcher<true>;
template class Searcher<false>;


