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

        std::atomic<uint64_t> masterNodes;
        std::array<AlignedAtomicU64, MAX_THREADS - 1> workerNodes;

        std::vector<std::thread> threadPool;
        Count workerCount;

        std::atomic<Count> workersReady;

        std::atomic<bool> destruct;
        std::atomic<bool> startgame;

        std::atomic<uint32_t> pulse;

        bool useWorkers; // use the threads (false in go nodes, ex)
        bool hasWorkers; // has more than 1 thread

        std::mutex mute;
        std::condition_variable sync;
        
        Engine();

        void newGame();
        void bench();

        void clearWorkerNodes(){ for (AlignedAtomicU64& a : workerNodes){ a.value = 0ULL; }}

        void runWorker(Count);
        void createPool(Count);
        void drainPool();

        template <bool> Score go(Depth, uint64_t, bool);
};