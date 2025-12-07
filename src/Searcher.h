// Class Definition for Searcher (a thread that searches)

#include "Evaluator.h"
#include "Generator.h"
#include "History.h"
#include "Princes.h"
#include "TeaTable.h"
#include "Timeman.h"

struct Sentry{
    bool nmp = false;
    Score presentEval;
    Move killer;
};

template <bool isMaster> class Searcher{
    public:
        Position pos;
        Generator gen;
        Evaluator eva;

        History his;

        std::array<Sentry, MAX_PLY + 1> sta;
        
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

        void invokeMove(const Move& m);
        void revokeMove(const Move& m);

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

        void scoreMoves(MoveList&, MoveScoreList&, const Index&, const Move&, const Index&);
        void scoreCaptures(MoveList&, MoveScoreList&, const Index&);
        void sortMoves(MoveList&, MoveScoreList&, const Index&);

        bool see(const Move&, const Score&);
        Score quiesce(Score, Score); //Index by Ply? - Could be useful for searchstack
        template <bool> Score alphabeta(Score, Score, Depth, Index);
        template <bool> Score search(Depth, uint64_t, uint64_t, bool);
        
        Score searchSilent(Depth d, uint64_t nl, uint64_t snl){ return search<false>(d, nl, snl, true); } // search with no output

        Score searchDepth(Depth d){ return searchSilent(d, ~0ULL, ~0ULL); } // bench
        Score searchInfinite(){ return searchDepth(MAX_PLY); } // worker thread
        Score searchSoftly(uint64_t snl){ return searchSilent(MAX_PLY, snl << 10, snl); } // datagen soft node limit

        void maybeForceStop();
        void disable(){ *stopSearch = true; }
        bool stopped(){ return *stopSearch; }

        void clearStack();
        void newGame();

        Move getBest(){ return bestMove; }
        //void reportBest(){ std::cout << "bestmove " << pos.moveName(bestMove) << std::endl; }
        
        const Score RFPbase = 40;
        const Score RFPmult = 60;
        const Depth maxRFPdepth = 6;
        const Depth minNMPdepth = 3;
        const Depth NMPreduce = 3;
        const Score NMPevalBase = 80;
        const Score NMPevalMult = 15;
        const int ASPbase = 30; //use int32 to really avoid overflows
        const int ASPmult = 2;
        const Depth minLMRdepth = 2;
        const double LMRbase = 0.4;
        const double LMRmult = 0.5;
        Table<Depth, MAX_PLY, 128> LMRtable;
        const std::array<Score, 6> SEEvals = {10000, 900, 500, 300, 300, 100};
        const Depth minIIRdepth = 3;
        const Depth IIRbase = 1;
        const Count LMPbase = 3;
        const Count LMPlin = 1;
        const Count LMPquad = 1;
        const Score FPbase = 200;
        const Score FPmult = 100;
        const Depth maxFPdepth = 5;
        //const Score SEEbase = 50;
        const Score SEElin = 80;
        const Depth maxSEEdepth = 7;
};

extern template class Searcher<true>;
extern template class Searcher<false>;

using Master = Searcher<true>;
using Worker = Searcher<false>;