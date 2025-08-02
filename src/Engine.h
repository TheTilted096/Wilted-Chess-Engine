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

        std::vector<std::thread> threadPool;
        Count workerCount; // number of workers

        std::atomic<Count> workersReady; // number of workers ready

        std::atomic<bool> destruct; // destroy all worker threads
        std::atomic<bool> startgame; //ucinewgame

        std::atomic<uint32_t> pulse; // signals new job for workers

        std::atomic<bool> useWorkers; // use the threads (false in go nodes, ex)
        bool hasWorkers; // has more than 1 thread

        std::mutex mute;
        std::condition_variable sync;
        std::condition_variable masterSync; // master waits on this condition variable
        
        Engine();
        ~Engine();

        void newGame();
        void bench();

        void clearWorkerNodes(){ for (AlignedAtomicU64& a : workerNodes){ a.value = 0ULL; }}

        void runWorker(Index);
        void createPool(Count);
        void drainPool();

        template <bool> Score go(Depth, uint64_t, bool);
};