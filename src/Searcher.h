// Class Definition for Searcher (a thread that searches)

#include "Evaluator.h"
#include "Generator.h"
#include "Princes.h"
#include "Timeman.h"

class Searcher{
    public:
        Position pos;
        Generator gen;
        Evaluator eva;
        
        Princes pvt;

        uint64_t nodes;
        uint64_t hardNodeMax;
        uint64_t lifeNodes;

        Move bestMove;   
        //Index movesDeep; //indicates how deep into search stack

        Timeman tim;

        Searcher();

        bool invokeMove(const Move& m);
        void revokeMove(const Move& m);

        void scoreMoves(MoveList&, std::array<uint32_t, MOVELIST_SIZE>&, const Index&);
        void scoreCaptures(MoveList&, std::array<uint32_t, MOVELIST_SIZE>&, const Index&);
        void sortMoves(MoveList&, std::array<uint32_t, MOVELIST_SIZE>&, const Index&);

        Score quiesce(Score, Score); //Index by Ply? - Could be useful for searchstack
        Score alphabeta(Score, Score, Depth, Index);
        Score search(Depth, uint64_t, bool);

        void maybeForceStop();

        void newGame();

        Move getBest(){ return bestMove; }

        void bench();

        static constexpr Score SCORE_INF = 21000;

        static constexpr Score VICTORY = 20000;
        static constexpr Score DEFEAT = -20000;
        static constexpr Score DRAW = 0;

        static constexpr Score FORCE_VICTORY = 19000;
        static constexpr Score FORCE_DEFEAT = -19000;

        static constexpr Index MAX_PLY = Princes::LEN; //maximum PV/search length
};