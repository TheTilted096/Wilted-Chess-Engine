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
        
        SharedArray<uint64_t>* nodesArrPtr;

        Move bestMove;

        Searcher();

        void assign(std::atomic<bool>*, TeaTable*, std::atomic<uint64_t>*);
        void promote(Princes*, Timeman*, SharedArray<uint64_t>*);

        void downloadPos(const Position& p){ pos = p; }

        bool invokeMove(const Move& m);
        void revokeMove(const Move& m);

        void scoreMoves(MoveList&, MoveScoreList&, const Index&, const Move&);
        void scoreCaptures(MoveList&, MoveScoreList&, const Index&);
        void sortMoves(MoveList&, MoveScoreList&, const Index&);

        Score quiesce(Score, Score); //Index by Ply? - Could be useful for searchstack
        template <bool> Score alphabeta(Score, Score, Depth, Index);
        template <bool> Score search(Depth, uint64_t, bool);

        Score searchInfinite(){ return search<false>(MAX_PLY, ~0ULL, true); }

        void maybeForceStop();
        void disable(){ *stopSearch = true; }

        void clearNodes(){ *nodesptr = 0ULL; }
        void addNode(){ (*nodesptr)++; }
        uint64_t nodes(){ return *nodesptr; }

        uint64_t pooledNodes(){ 
            uint64_t s = nodes(); 
            for (AlignedAtomicU64& a : *nodesArrPtr){
                s += a.value;
            }
            return s;
        }
    
        void newGame();

        Move getBest(){ return bestMove; }
};

extern template class Searcher<true>;
extern template class Searcher<false>;

using Master = Searcher<true>;
using Worker = Searcher<false>;