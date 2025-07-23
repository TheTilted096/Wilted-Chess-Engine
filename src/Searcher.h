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

        //bool* stopSearch;
        std::atomic<bool>* stopSearch; //thread-safe writes

        TeaTable* ttref;

        std::atomic<uint64_t>* nodesptr; //holds nodecount
        std::vector<std::atomic<uint64_t>>* nodesVecPtr; //used by master thread to read node counts of workers

        uint64_t hardNodeMax; 
        //only master thread uses this, but all threads have it nonetheless...unfortunately

        Move bestMove;

        Searcher();

        void assign(std::atomic<uint64_t>*, std::atomic<bool>*, TeaTable*);   
        void promote(Princes*, Timeman*, std::vector<std::atomic<uint64_t>>*);
        
        uint64_t nodes(){ return *nodesptr; }
        void clearNodes(){ *nodesptr = 0ULL; }
        void addNode(){ (*nodesptr)++; }

        bool invokeMove(const Move& m);
        void revokeMove(const Move& m);

        void scoreMoves(MoveList&, std::array<uint32_t, MOVELIST_SIZE>&, const Index&, const Move&);
        void scoreCaptures(MoveList&, std::array<uint32_t, MOVELIST_SIZE>&, const Index&);
        void sortMoves(MoveList&, std::array<uint32_t, MOVELIST_SIZE>&, const Index&);

        Score quiesce(Score, Score); //Index by Ply? - Could be useful for searchstack
        template <bool> Score alphabeta(Score, Score, Depth, Index);
        template <bool> Score search(Depth, uint64_t, bool);

        void maybeForceStop();

        void newGame();

        Move getBest(){ return bestMove; }
};

extern template class Searcher<true>;
extern template class Searcher<false>;

using Master = Searcher<true>;
using Worker = Searcher<false>;