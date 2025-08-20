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
        SharedArray<uint64_t> workerNodes;

        std::vector<std::thread> workerPool;
        Count workerCount;

        std::atomic<Count> workersReady;
        
        std::atomic<bool> destruct; // destroy all worker threads
        std::atomic<bool> startgame; // ucinewgame

        std::atomic<uint32_t> pulse;
        std::atomic<bool> useWorkers; // use the threads (false in go nodes, for ex)
        bool hasWorkers; // more than 1 thread exists

        std::mutex mute;
        std::condition_variable sync;
        std::condition_variable masterSync;
        
        Engine();
        ~Engine();

        void newGame();
        void bench();

        void clearWorkerNodes(){ for (AlignedAtomicU64& a : workerNodes){ a.value = 0ULL; }}

        void runWorker(Index);
        void createPool(Count);
        void drainPool();

        void workerPing(); // signal ready to master
        void masterIdle(); // make master wait for workers
        
        template <bool> Score go(Depth, uint64_t, uint64_t, bool);
        Score goSoftly(uint64_t);
        Score goDepth(Depth);

        Move bestMove(){ return master.getBest(); }
};