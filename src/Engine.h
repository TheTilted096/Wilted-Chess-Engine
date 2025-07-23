// Class Definition for Engine

#include "Searcher.h"

class Engine{
    public:
        Master master;

        TeaTable ttable;

        Princes pvtable;
        Timeman timer;

        std::atomic<bool> stopFlag;
        
        Count workCount;
        std::vector<std::atomic<uint64_t>> workerNodes;
        std::atomic<uint64_t> masterNodes;
        
        Engine();

        void newGame();
        void bench();

        template <bool> Score go(Depth, uint64_t, bool);
};