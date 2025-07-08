// Class Definition for Engine

#include "Searcher.h"

class Engine{
    public:
        //TeaTable ttable;

        Searcher master;
        
        Engine();

        void newGame();
        void bench();

        Score go(Depth, uint64_t, bool);
};