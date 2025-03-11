/*
Class Definition of Engine
Full Search Object

TheTilted096 12-18-24

*/

#include "WiltedPosition.cpp"

class TTentry{
    public:
        uint64_t data; //move 0-31, score 32-47, depth 48-53, nodetype 54-55, flag 56, age(?)
        Hash hash;

        int eScore();
        int enType();
        int eDepth();
        Hash eHash();
        Move eMove();
        //Move eMove;

        TTentry();
        void update(int, int, int, Move, Hash);
        void reset();
        //void print();
};

TTentry::TTentry(){
    reset();
}

void TTentry::update(int scr, int ntype, int dep, Move m, Hash h){
    data = m.info;
    data |= (((uint64_t) ((uint16_t) (scr))) << 32);
    data |= (((uint64_t) dep) << 48);
    data |= (((uint64_t) ntype) << 54);

    data &= ~(1ULL << 56);

    hash = h;
}

int TTentry::eScore(){
    return ((int) ((int16_t) ((data >> 32) & 0xFFFF)));
}

int TTentry::enType(){
    return ((data >> 54) & 0x3);
}

int TTentry::eDepth(){
    return ((data >> 48) & 0x3F);
}

Move TTentry::eMove(){
    return Move(data & 0xFFFFFFFF);
}

void TTentry::reset(){
    data = (1ULL << 56);
}

Hash TTentry::eHash(){
    return hash;
}

class Sentry{ //search stack entry
    public:
        bool nmp;
        int presentEval;
        //Move killer;

        Sentry();
        void clear();
};

Sentry::Sentry(){
    clear();
}

void Sentry::clear(){
    presentEval = -29501;
    nmp = false;
}

class Engine : public Position{
    public:
        Move bestMove;

        uint64_t mnodes;
        //uint64_t nodesForever;

        //Tranposition Table
        TTentry* ttable;

        //Search Stack
        Sentry stack[64];

        bool timeKept;
        int64_t thinkLimit;
        uint32_t hardTime, softTime;
        std::chrono::_V2::steady_clock::time_point moment;

        //Tunables
        const int minNMPdepth = 2;
        const int NMPreduce = 2;
        const int ASPbase = 50;
        const int ASPmult = 2;
        const int maxRFPdepth = 6;
        const int RFPbase = 40;
        const int RFPmult = 60;
        const int minLMRdepth = 2;
        const double LMRbase = 0.4;
        const double LMRmult = 0.6;

        int LMRreds[64][128];

        Engine();
        ~Engine();

        void forceStop();
        int64_t timeTaken();
        void timeMan(uint32_t, uint32_t);

        void eraseTransposeTable();
        void eraseStack();

        void scoreMoves(int, int, Move);
        void scoreQMoves(int, int);

        void sortMoves(int, int);

        int quiesce(int, int, int);
        int alphabeta(int, int, int, int);
        int search(int, uint64_t, bool);

        void newGame();

        Move getMove(){ return bestMove; }

};

Engine::Engine(){
    //maximum nodes
    mnodes = 0ULL;
    hardTime = ~0U; softTime = ~0U;

    timeKept = false;

    //init transposition table
    ttable = new TTentry[0x100000]; // (1 << 20)

    //LMR Coefs
    for (int i = 0; i < 64; i++){
        for (int j = 0; j < 128; j++){
            LMRreds[i][j] = LMRbase + LMRmult * log(i + 1) * log(j + 1);
        }
    }

    //just ucinewgame
    newGame();
}

Engine::~Engine(){
    delete[] ttable;
}

void Engine::eraseTransposeTable(){
    for (int i = 0; i < 0xFFFFF; i++){
        ttable[i].reset();
    }
}

void Engine::eraseStack(){
    for (int i = 0; i < 64; i++){
        stack[i].clear();
    }
}

void Engine::forceStop(){
    if (timeKept and (nodes % 2048 == 0)){
        if (timeTaken() > hardTime){
            throw "Time Expired\n";
        }
    }

    if (nodes > mnodes){
        throw "Nodes Exceeded\n";
    }
}

int64_t Engine::timeTaken(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - moment).count();
}

void Engine::timeMan(uint32_t base, uint32_t inc){
    timeKept = ~base;
    softTime = base / 40 + inc / 2;
    hardTime = std::min(9 * base / 10, base / 10 + 9 * inc / 10);
}

void Engine::scoreMoves(int ply, int nc, Move ttm){
    for (int i = 0; i < nc; i++){
        //TT Move, Killers

        if (moves[ply][i] == ttm){
            mprior[ply][i] = (1 << 30);
            continue;
        }

        if (moves[ply][i].gcptp()){ //captures
            mprior[ply][i] = (1 << 16) + moves[ply][i].gtpmv() - (moves[ply][i].gcptp() << 4);
            continue;
        }

        mprior[ply][i] = moves[ply][i].gtpmv();        
    }
}

void Engine::scoreQMoves(int ply, int nc){
    for (int i = 0; i < nc; i++){

        mprior[ply][i] = (1 << 16) + moves[ply][i].gtpmv() - (moves[ply][i].gcptp() << 4);

    }
}

void Engine::sortMoves(int ply, int nc){
    int keyVal;
    Move keyMove;

    int j;
    for (int i = 1; i < nc; i++){
        keyVal = mprior[ply][i];
        keyMove = moves[ply][i];
        j = i - 1;
        while ((j >= 0) and (mprior[ply][j] < keyVal)){
            mprior[ply][j + 1] = mprior[ply][j];
            moves[ply][j + 1] = moves[ply][j];
            j--;
        }

        mprior[ply][j + 1] = keyVal;
        moves[ply][j + 1] = keyMove;
    }
}

int Engine::quiesce(int alpha, int beta, int ply){
    /*
    Check Insufficient Material
    */
    int score = evaluate();

    int bestScore = score;

    /*
    if (ply > 31){ //cap to prevent segfaults
        return score;
    }
    */

    if (score >= beta){
        return score;
    }
    if (alpha < score){
        alpha = score;
    }

    int lply = 64 + ply;
    int moveCount = generateCaptures(lply);

    scoreQMoves(lply, moveCount);
    sortMoves(lply, moveCount);

    for (int i = 0; i < moveCount; i++){
        makeMove<true>(moves[lply][i]);

        forceStop();

        if (isChecked(!toMove)){
            unmakeMove<true>(moves[lply][i]);
            continue;
        }

        score = -quiesce(-beta, -alpha, ply + 1);
        unmakeMove<true>(moves[lply][i]);

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

int Engine::alphabeta(int alpha, int beta, int depth, int ply){
    int score = -29000;
    int bestScore = -29000;

    //3-fold
    int repeats = countReps(ply);
    if (repeats > 2){
        return 0;
    } 

    //50-move rule
    if (chm[thm] >= 100){
        return 0;
    }

    //insufficient material
    if (insufficient()){
        return 0;
    }

    if (depth == 0){
        return quiesce(alpha, beta, 0);
        //return evaluate();
    }

    bool inCheck = isChecked(toMove);


    bool isPV = (beta - alpha > 1);

    //Mate Distance Pruning
    if (!isPV){
        int ma = std::max(alpha, -29000 + ply);
        int mb = std::min(beta, 29000 - ply - 1);

        if (ma >= mb){
            return ma;
        }
    }

    //Transposition Table Probing
    int ttindex = zhist[thm] & 0xFFFFF;
    int ttdepth = ttable[ttindex].eDepth();
    Move ttmove;

    if (ttable[ttindex].eHash() == zhist[thm]){
        score = ttable[ttindex].eScore();
        ttmove = ttable[ttindex].eMove();

        //implement TT Cutoffs
        int ntype = ttable[ttindex].enType();
        if ((ttdepth >= depth) and (repeats == 1) and !isPV and (ply > 0)){
            if (ntype == 1){
                return score;
            }
            if ((ntype == 2) and (score >= beta)){
                return score;
            }
            if ((ntype == 3) and (score <= alpha)){
                return score;
            }
        }
    }


    stack[ply].presentEval = evaluate();

    if (ply > 0){ //template <rootNode> someday...
        //RFP
        int margin = RFPbase + RFPmult * depth;
        if ((depth < maxRFPdepth) and !inCheck
                and (stack[ply].presentEval - beta > margin)){

            return (stack[ply].presentEval + beta) / 2;
        }


        //NMP
        stack[ply].nmp = !stack[ply - 1].nmp and (depth > minNMPdepth) 
                and !onlyPawns(toMove) and !inCheck /*and (beta > -27000)*/;
        
        if (stack[ply].nmp){
            passMove();
            int nullScore = -alphabeta(-beta, -alpha, depth - 1 - NMPreduce, ply + 1); //reduction = 2
            unpassMove();
            if (nullScore >= beta /*and nullScore < 27000*/){
                //assert(abs(score) < 27000);
                return nullScore;
            }
        }

    }


    int moveCount = generateMoves(ply);

    //Move Scoring/Ordering
    scoreMoves(ply, moveCount, ttmove);
    sortMoves(ply, moveCount);

    bool isAllNode = true; //used for TT updating
    Move localBestMove;

    bool noisyMove;

    for (int i = 0; i < moveCount; i++){
        makeMove<true>(moves[ply][i]);

        forceStop(); //forcefully quits search if needed

        if (notValid(moves[ply][i])){
            unmakeMove<true>(moves[ply][i]);
            continue;
        }

        noisyMove = moves[ply][i].gcptp(); //captures noisy for now

        //Shallow Depth Pruning

        //score = -alphabeta(-beta, -alpha, depth - 1, ply + 1); no PVS

        //PVS, LMR
        if (i == 0){ //First Move
            score = -alphabeta(-beta, -alpha, depth - 1, ply + 1); //Search Full Window
        } else {
            // Just PVS
            /*
            score = -alphabeta(-alpha - 1, -alpha, depth - 1, ply + 1); //Otherwise, search with null window

            if ((score > alpha) and isPV){
                score = -alphabeta(-beta, -alpha, depth - 1, ply + 1);
            }
            */
            
            int r = 0;
            if (depth > minLMRdepth and !noisyMove){ //determine reduction
                r = LMRreds[ply][i];
            }

            //perform reduced search
            score = -alphabeta(-alpha - 1, -alpha, std::max(0, depth - 1 - r), ply + 1);

            //if reduced depth and surprise, re-search at full depth
            if (score > alpha and r > 0){
                score = -alphabeta(-alpha - 1, -alpha, depth - 1, ply + 1);
            }

            //if further surprise, research fully. guard with isPV to avoid redundant null-windows.
            if (score > alpha and isPV){ 
                score = -alphabeta(-beta, -alpha, depth - 1, ply + 1);
            }
        }

        unmakeMove<true>(moves[ply][i]);

        if (score >= beta){
            //TT update in cut node
            ttable[ttindex].update(score, 2, depth, moves[ply][i], zhist[thm]);

            //Killers and History

            return score;
        }

        if (score > alpha){ //PV Node
            //No longer all node
            isAllNode = false;

            alpha = score;
        }

        if (score > bestScore){
            if (ply == 0){
                bestMove = moves[ply][i];
            }

            //local best move
            localBestMove = moves[ply][i];
            bestScore = score;
        }
    }

    //assert(bestScore > -29000);
    if (score == -29000){
        //print();
        //maybe make this branchless
        if (inCheck){ return ply - 29000; } //dying
        return 0; //draw
    }

    ttable[ttindex].update(bestScore, 1 + 2 * isAllNode, depth, localBestMove, zhist[thm]);

    //assert(bestScore > -29000);
    return bestScore;
}

int Engine::search(int mdepth, uint64_t maxNodes, bool output){
    //save position
    Bitboard oPos[8] = {sides[0], sides[1], pieces[0], pieces[1], pieces[2], pieces[3], pieces[4], pieces[5]};
    bool oMove = toMove; int othm = thm;

    //init alpha, beta
    int alpha = -30000, beta = 30000;
    
    //erase history table

    //begin mobilities and attack tables (might be looped into evalscratch);
    evaluateScratch();

    //aspiration windows
    int searchScore;
    int prevScore;
    Move currentBest(0);

    bool aspFail;

    //set time and node counts
    nodes = 0;
    mnodes = maxNodes;
    moment = std::chrono::steady_clock::now();

    try {
        searchScore = alphabeta(alpha, beta, 0, 0); //depth 0 search
        prevScore = searchScore;

        if (output){ std::cout << "info depth 0 nodes " << nodes << " score cp " << searchScore << std::endl; }

        for (int i = 1; i <= mdepth; i++){ //iterative deepening search
            /*
            searchScore = alphabeta(-30000, 30000, i, 0);
            
            currentBest = bestMove; //each complete depth saves best move and score
            if (output){ std::cout << "info depth " << i << " nodes " << nodes << " score cp " << searchScore << std::endl; }

            if (timeKept and (timeTaken() > softTime)){ //quit search if exceeded soft time
                break;
            }
            */
            aspFail = true;
            int aspa = ASPbase, aspb = ASPbase;
            
            while (aspFail){
                alpha = prevScore - aspa;
                beta = prevScore + aspb;

                searchScore = alphabeta(alpha, beta, i, 0);

                if (searchScore <= alpha){
                    aspa *= ASPmult;
                } else if (searchScore >= beta){
                    aspb *= ASPmult;
                } else {
                    aspFail = false;
                }
            }

            currentBest = bestMove;
            prevScore = searchScore;
            if (output){ std::cout << "info depth " << i << " nodes " << nodes << " score cp " << searchScore << std::endl; }
            

            if (timeKept and (timeTaken() > softTime)){ //quit search if exceeded soft time
                break;
            }
        }

    } catch (const char* e){
        bestMove = currentBest; //when hard-stopped, reset all the pieces back
    }

    auto end = std::chrono::steady_clock::now();
    double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - moment).count();

    if (output){
        std::cout << "info nodes " << nodes << " nps ";
        std::cout << (int) ((nodes / dur) * 1000000) << '\n';

        std::cout << "bestmove " << bestMove.toStr() << '\n';
    }

    //nodesForever += nodes;

    //reload position
    sides[0] = oPos[0]; sides[1] = oPos[1];
    pieces[0] = oPos[2]; pieces[1] = oPos[3]; pieces[2] = oPos[4]; 
    pieces[3] = oPos[5]; pieces[4] = oPos[6]; pieces[5] = oPos[7];
    thm = othm; toMove = oMove;

    //reset nodeCount, thinkLimit, maxnodes
    nodes = 0ULL;
    mnodes = ~0ULL;


    return prevScore;
}

void Engine::newGame(){
    setStartPos();

    //erase history, killers, TT
    eraseTransposeTable();
    eraseStack();

    beginZobrist();
}




















