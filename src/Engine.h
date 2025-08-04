// Class Definition for Engine

#include "Searcher.h"

class Engine{
    public:
        Master master;

        Position mainpos;
        Generator maingen;

        TeaTable ttable;

        Princes pvtable;
        Timeman timer;
        std::atomic<bool> stopFlag;

        AlignedAtomicU64 masterNodes;
        
        Engine();

        void newGame();
        void bench();

        template <bool> Score go(Depth, uint64_t, bool);
};