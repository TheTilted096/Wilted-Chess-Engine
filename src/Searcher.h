// Class Definition for Searcher (a thread that searches)

#include "Evaluator.h"
#include "Generator.h"

class PrinceTable;

class Searcher{
    public:
        Position pos;
        Generator gen;
        Evaluator eva;

        PrinceTable pvt;

        uint64_t nodes;
        //uint64_t hardNodeCap;

        Move bestMove;   
        //Index movesDeep; //indicates how deep into search stack

        std::chrono::steady_clock::time_point moment; //timed search start point

        Searcher();

        bool invokeMove(const Move& m);
        void revokeMove(const Move& m);

        Score alphabeta(Score, Score, Depth, Index);
        Score search();

        void newGame();

        Move getBest(){ return bestMove; }

        static constexpr Score SCORE_INF = 21000;

        static constexpr Score VICTORY = 20000;
        static constexpr Score DEFEAT = -20000;
        static constexpr Score DRAW = 0;

        static constexpr Score FORCE_VICTORY = 19000;
        static constexpr Score FORCE_DEFEAT = -19000;

        static constexpr Index MAX_PLY = 16;
};

class PrinceTable{
    
};