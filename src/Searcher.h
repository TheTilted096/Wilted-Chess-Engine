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
        template <bool> Score alphabeta(Score, Score, Depth, Index);
        Score search(Depth, uint64_t, bool);

        void maybeForceStop();

        void newGame();

        Move getBest(){ return bestMove; }

        void bench();
};