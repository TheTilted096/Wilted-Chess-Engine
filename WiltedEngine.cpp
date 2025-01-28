/*
Class Definition of Engine
Full Search Object

TheTilted096 12-18-24

*/

#include "WiltedPosition.cpp"

/*
class TTentry{
    public:
        uint64_t data;
        
        Hash eHash;
        Hash* zref;

};

*/

class Engine : public Position{
    public:
        Move bestMove;

        uint64_t mnodes;
        //uint64_t nodesForever;

        //Tranposition Table
        //Killer Moves

        //bool timeKept;
        //int64_t thinkLimit;
        std::chrono::_V2::steady_clock::time_point moment;

        Engine();

        //int quiesce(int, int, int);
        int alphabeta(int, int, int, int);
        int search();

        void newGame();

};

Engine::Engine(){
    //maximum nodes

    //init transposition table

    //LMR Coefs

    //just ucinewgame
}

int Engine::alphabeta(int alpha, int beta, int depth, int ply){
    int score = -29000;
    int bestScore = -29000;

    //3-fold 

    //50-move rule

    //insufficient material

    if (depth == 0){
        return evaluate();
    }

    //Mate Distance Pruning

    //Transposition Table Probing

    //RFP

    //NMP


    int moveCount = generateMoves(ply);

    bool inCheck = isChecked(toMove);

    //Move Scoring/Ordering

    for (int i = 0; i < moveCount; i++){
        makeMove<true>(moves[ply][i]);

        //endHandle();

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
        //maybe make this branchless
        if (inCheck){ return ply - 29000; } //dying
        return 0; //draw
    }


    return bestScore;
}

int Engine::search(){
    //save position

    //init alpha, beta

    //current best move and evaluation

    //erase history table
    evaluateScratch();

    //begin mobilities and attack tables (might be looped into evalscratch);

    //aspiration windows
    int searchScore;

    //set time and node counts
    //thinkLimit = thinkTime;
    nodes = 0;
    //mnodes = maxNodes;
    moment = std::chrono::steady_clock::now();

    searchScore = alphabeta(-30000, 30000, 4, 0);

    std::cout << "info depth 4 nodes " << nodes << " score cp " << searchScore << '\n';

    auto end = std::chrono::steady_clock::now();
    double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - moment).count();

    std::cout << "info nodes " << nodes << " nps ";
    std::cout << (int) ((nodes / dur) * 1000000) << '\n';

    std::cout << "bestmove " << bestMove.toStr() << '\n';

    //nodesForever += nodes;

    //reload position
    //reset nodeCount, thinkLimit, maxnodes


    return searchScore;   
}

void Engine::newGame(){
    setStartPos();
}






















