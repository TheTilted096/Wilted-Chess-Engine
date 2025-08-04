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
        bool* stopSearch;

        TeaTable* ttref;

        uint64_t nodes;
        uint64_t hardNodeMax;

        Move bestMove;

        Searcher();

        void assign(Princes*, Timeman*, bool*, TeaTable*);

        bool invokeMove(const Move& m);
        void revokeMove(const Move& m);

        void scoreMoves(MoveList&, MoveScoreList&, const Index&, const Move&, const Index&);
        void scoreCaptures(MoveList&, MoveScoreList&, const Index&);
        void sortMoves(MoveList&, MoveScoreList&, const Index&);

        Score quiesce(Score, Score); //Index by Ply? - Could be useful for searchstack
        template <bool> Score alphabeta(Score, Score, Depth, Index);
        template <bool> Score search(Depth, uint64_t, bool);

        void maybeForceStop();
        void disable(){ *stopSearch = true; }

        void clearStack();
        void newGame();

        Move getBest(){ return bestMove; }
        
        const Score RFPbase = 40;
        const Score RFPmult = 60;
        const Depth maxRFPdepth = 6;
        const Depth minNMPdepth = 3;
        const Depth NMPreduce = 3;
        const int ASPbase = 50; //use int32 to really avoid overflows
        const int ASPmult = 2;
        const Depth minLMRdepth = 2;
        const double LMRbase = 0.4;
        const double LMRmult = 0.6;
        Table<Depth, MAX_PLY, 128> LMRtable;
};

extern template class Searcher<true>;
extern template class Searcher<false>;

using Master = Searcher<true>;
using Worker = Searcher<false>;