// Class Definition for Searcher (a thread that searches)

#include "Evaluator.h"
#include "Generator.h"
#include "Princes.h"
#include "TeaTable.h"
#include "Timeman.h"

template <bool isMaster> class Searcher{
    public:
        Position pos;
        Generator gen;
        Evaluator eva;
        
        //shared items, only modified by master
        Princes* pvt;
        Timeman* tim;
        std::atomic<bool>* stopSearch;

        TeaTable* ttref;

        std::atomic<uint64_t>* nodesptr;
        uint64_t hardNodeMax;

        Move bestMove;

        Searcher();

        void assign(std::atomic<bool>*, TeaTable*, std::atomic<uint64_t>*);
        void promote(Princes*, Timeman*);

        void downloadPos(const Position& p){ pos = p; }

        bool invokeMove(const Move& m);
        void revokeMove(const Move& m);

        void scoreMoves(MoveList&, MoveScoreList&, const Index&, const Move&);
        void scoreCaptures(MoveList&, MoveScoreList&, const Index&);
        void sortMoves(MoveList&, MoveScoreList&, const Index&);

        Score quiesce(Score, Score); //Index by Ply? - Could be useful for searchstack
        template <bool> Score alphabeta(Score, Score, Depth, Index);
        template <bool> Score search(Depth, uint64_t, bool);

        void maybeForceStop();
        void disable(){ *stopSearch = true; }

        void clearNodes(){ *nodesptr = 0ULL; }
        void addNode(){ (*nodesptr)++; }
        uint64_t nodes(){ return *nodesptr; }
    
        void newGame();

        Move getBest(){ return bestMove; }
};

extern template class Searcher<true>;
extern template class Searcher<false>;

using Master = Searcher<true>;
using Worker = Searcher<false>;