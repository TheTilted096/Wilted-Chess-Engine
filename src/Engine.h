// Class Definition for Engine

#include "Searcher.h"

class Engine{
    public:
        //TeaTable ttable;

        Master master;

        Princes pvtable;
        Timeman timer;
        bool stopFlag;
        
        Engine();

        void newGame();
        void bench();

        template <bool> Score go(Depth, uint64_t);
};