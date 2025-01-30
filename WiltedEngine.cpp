/*
Class Definition of Engine
Full Search Object

TheTilted096 12-18-24

*/

#include "WiltedPosition.cpp"

class TTentry{
    public:
        uint64_t data; //move 0-31, score 32-47, depth 
        uint64_t hash;
        uint64_t* zref; //reference list

        int eScore();
        int enType();
        int eDepth();
        Move eMove();
        //Move eMove;

        TTentry();
        void setRef(uint64_t*);
        void update(int, int, int, Move, int);
        void reset();
        //void print();
};



class Engine : public Position{
    public:
        Move bestMove;

        uint64_t mnodes;
        //uint64_t nodesForever;

        //Tranposition Table
        //TTentry* ttable;

        //Killer Moves

        bool timeKept;
        int64_t thinkLimit;
        uint32_t hardTime, softTime;
        std::chrono::_V2::steady_clock::time_point moment;

        Engine();
        //~Engine();

        void forceStop();
        int64_t timeTaken();
        void timeMan(uint32_t, uint32_t);

        void scoreMoves(int, int);
        void sortMoves(int, int);

        int quiesce(int, int, int);
        int alphabeta(int, int, int, int);
        int search(int, uint64_t, bool);

        void newGame();

};

Engine::Engine(){
    //maximum nodes
    mnodes = 0ULL;
    hardTime = ~0U; softTime = ~0U;

    //init transposition table
    /*
    ttable = new TTentry[0x100000]; // (1 << 20)
    for (int i = 0; i < 0x100000; i++){
        ttable[i].setRef(zhist);
    }
    */

    //LMR Coefs

    //just ucinewgame
    newGame();
}

/*
Engine::~Engine(){
    //delete[] ttable;
}
*/

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
    hardTime = base / 10 + 9 * inc / 10;
}

void Engine::scoreMoves(int ply, int nc){
    for (int i = 0; i < nc; i++){
        //TT Move, Killers

        if (moves[ply][i].capt()){ //captures
            mprior[ply][i] = (1 << 16) + moves[ply][i].tpmv() - (moves[ply][i].cptp() << 4);
            continue;
        }

        mprior[ply][i] = moves[ply][i].tpmv();        
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

    if (ply > 15){ //cap to prevent segfaults
        return score;
    }

    if (score >= beta){
        return score;
    }
    if (alpha < score){
        alpha = score;
    }

    int lply = 48 + ply;
    int moveCount = generateCaptures(lply);

    scoreMoves(ply, moveCount);
    sortMoves(ply, moveCount);

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

    if (depth == 0){
        return quiesce(alpha, beta, 0);
        //return evaluate();
    }

    //Mate Distance Pruning

    //Transposition Table Probing

    //RFP

    //NMP


    int moveCount = generateMoves(ply);

    bool inCheck = isChecked(toMove);

    //Move Scoring/Ordering
    scoreMoves(ply, moveCount);
    sortMoves(ply, moveCount);

    for (int i = 0; i < moveCount; i++){
        makeMove<true>(moves[ply][i]);

        forceStop(); //forcefully quits search if needed

        if (notValid(moves[ply][i])){
            unmakeMove<true>(moves[ply][i]);
            continue;
        }

        //Shallow Depth Pruning

        score = -alphabeta(-beta, -alpha, depth - 1, ply + 1);

        //PVS is the scourge of the century
        //so is LMR

        unmakeMove<true>(moves[ply][i]);

        if (score >= beta){
            //TT update in cut node

            //Killers and History

            return score;
        }

        if (score > alpha){
            //No longer all node

            alpha = score;
        }

        if (score > bestScore){
            if (ply == 0){
                bestMove = moves[ply][i];
            }

            //local best move

            bestScore = score;
        }
    }

    if (score == -29000){
        //print();
        //maybe make this branchless
        if (inCheck){ return ply - 29000; } //dying
        return 0; //draw
    }


    return bestScore;
}

int Engine::search(int mdepth, uint64_t maxNodes, bool output){
    //save position
    Bitboard oPos[8] = {sides[0], sides[1], pieces[0], pieces[1], pieces[2], pieces[3], pieces[4], pieces[5]};
    bool oMove = toMove; int othm = thm;

    //init alpha, beta

    //current best move and evaluation
    Move currentBest(0);
    
    //erase history table

    evaluateScratch();

    //begin mobilities and attack tables (might be looped into evalscratch);

    //aspiration windows
    int searchScore;

    //set time and node counts
    nodes = 0;
    mnodes = maxNodes;
    moment = std::chrono::steady_clock::now();

    //searchScore = alphabeta(-30000, 30000, 4, 0);

    //std::cout << "info depth 4 nodes " << nodes << " score cp " << searchScore << '\n';

    try {
        searchScore = alphabeta(-30000, 30000, 0, 0); //depth 0 search

        if (output){ std::cout << "info depth 0 nodes " << nodes << " score cp " << searchScore << std::endl; }

        for (int i = 1; i <= mdepth; i++){ //iterative deepening search
            searchScore = alphabeta(-30000, 30000, i, 0);
            

            currentBest = bestMove; //each complete depth saves best move and score
            if (output){ std::cout << "info depth " << i << " nodes " << nodes << " score cp " << searchScore << std::endl; }

            if (timeKept and (timeTaken() > softTime)){ //quit search if exceeded soft time
                //std::cout << "Soft Time Reached\n";
                break;
            }
        }

    } catch (const char* e){
        //std::cout << e << '\n';
        bestMove = currentBest; //when hard-stopped, reset all the pieces back
    }

    auto end = std::chrono::steady_clock::now();
    double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - moment).count();

    std::cout << "info nodes " << nodes << " nps ";
    std::cout << (int) ((nodes / dur) * 1000000) << '\n';

    std::cout << "bestmove " << bestMove.toStr() << '\n';

    //nodesForever += nodes;

    //reload position
    sides[0] = oPos[0]; sides[1] = oPos[1];
    pieces[0] = oPos[2]; pieces[1] = oPos[3]; pieces[2] = oPos[4]; 
    pieces[3] = oPos[5]; pieces[4] = oPos[6]; pieces[5] = oPos[7];
    thm = othm; toMove = oMove;

    //reset nodeCount, thinkLimit, maxnodes
    nodes = 0ULL;
    mnodes = ~0ULL;


    return searchScore;   
}

void Engine::newGame(){
    setStartPos();

    //erase history, killers, TT

    beginZobrist();
}




















