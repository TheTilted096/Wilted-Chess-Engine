// Function Definitions for Engine

#include "Engine.h"

Engine::Engine(){
    stopFlag = false;

    destruct = false;
    startgame = false;
    pulse = 0U;
    workerCount = 0;
    workersReady = 0;
    useWorkers = true;
    hasWorkers = false;

    masterNodes.value = 0ULL;
    clearWorkerNodes();

    maingen.assign(&mainpos);

    master.assign(&stopFlag, &ttable, &masterNodes.value);
    master.promote(&pvtable, &timer, &workerNodes);

    newGame();
}

Engine::~Engine(){
    drainPool();
}

void Engine::newGame(){
    ttable.clear();
    pvtable.clearAll();
    master.newGame();

    mainpos.setStartPos();

    if (hasWorkers){
        {
            std::lock_guard<std::mutex> guard(mute);
            startgame = true;
            //assert(workersReady == workerCount);
            workersReady = 0;
            pulse++;
        }

        sync.notify_all();
        
        masterIdle();
        //assert(workersReady == workerCount);

        startgame = false;
    }
}

void Engine::bench(){
    std::string marks[8] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r1bqkb1r/2p2ppp/p1np1n2/1p2p3/4P3/1B3N2/PPPP1PPP/RNBQR1K1 b kq - 1 7",
        "3r2k1/2pr2p1/p2bpn1p/Pp2p3/1Pq1P3/2PR1N1P/2Q2PP1/2B1R1K1 w - - 5 28",
        "2r1qrk1/1b1n2p1/1p1ppn1p/p1p2p2/P1PP4/2PBPP2/2Q2NPP/2B1RRK1 b - - 1 10",
        "6k1/5pp1/4n3/3p4/r6Q/6P1/q4PBP/3R3K w - - 0 35",
        "6k1/6p1/5p1p/pR1b4/P1r5/4PN2/2P3PP/6K1 b - - 2 35",
        "8/8/5k2/8/8/6P1/6K1/8 w - - 0 57",
        "6k1/8/6p1/8/8/6P1/4qQKP/8 b - - 5 49"
    };

    //auto benchStart = std::chrono::steady_clock::now();

    uint64_t lifeNodes = 0ULL;

    timer.start();

    for (std::string tester : marks){
        newGame();
        
        mainpos.readFen(tester);
        goDepth(14);
        lifeNodes += master.nodes();
    }

    //auto benchEnd = std::chrono::steady_clock::now();
    //int64_t dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(benchEnd - benchStart).count();
    int64_t dur = timer.elapsed();
    int64_t nps = 1000000 * lifeNodes / dur;

    std::cout << lifeNodes << " nodes " << nps << " nps " << std::endl;
}

template <bool out> Score Engine::go(Depth d, uint64_t nl, uint64_t snl, bool mp){
    /* old singlethreaded code
    stopFlag = false;
    timer.start();

    d = std::min(MAX_PLY, d);

    master.downloadPos(mainpos);
    master.clearNodes();

    Score result = master.search<out>(d, nl, mp);

    return result;
    */

    useWorkers = (nl == ~0ULL) and (snl == ~0ULL);

    if (hasWorkers and useWorkers){
        {
            std::lock_guard<std::mutex> guard(mute);
            stopFlag = false;
            clearWorkerNodes();
            //assert(workersReady == workerCount);
            workersReady = 0;
            pulse++;
        }
        sync.notify_all();
    } else {
        stopFlag = false;
    }

    d = std::min(MAX_PLY, d);

    master.downloadPos(mainpos);
    master.clearNodes();

    timer.start();

    Score sc = master.search<out>(d, nl, snl, mp);

    if (hasWorkers and useWorkers){
        masterIdle();
        //assert(workerCount == workersReady);
    }

    master.reportBest();

    return sc;
}

Score Engine::goSoftly(uint64_t snl){
    useWorkers = false;

    stopFlag = false;
    master.downloadPos(mainpos);
    master.clearNodes();

    Score sc = master.searchSoftly(snl);

    return sc;
}

Score Engine::goDepth(Depth d){
    useWorkers = false;

    stopFlag = false;
    master.downloadPos(mainpos);
    master.clearNodes();

    Score sc = master.searchDepth(d);

    return sc;
}

void Engine::masterIdle(){
    std::unique_lock<std::mutex> lock(mute);
    masterSync.wait(lock, [&]{ return (workersReady == workerCount); });
}

void Engine::runWorker(Index id){
    Worker w;
    w.assign(&stopFlag, &ttable, &workerNodes[id].value);

    uint32_t lastPulse;
    {
        std::lock_guard<std::mutex> guard(mute);
        lastPulse = pulse;
        workersReady++;
    }

    masterSync.notify_one();

    while (true){
        {
            std::unique_lock<std::mutex> lock(mute);
            sync.wait(lock, [&]{ return (pulse > lastPulse); });
            lastPulse = pulse;
        }

        if (destruct){
            return;
        }

        if (startgame){ //ucinewgame
            w.newGame();
            workerPing();
            masterSync.notify_one(); //only one thread; master
            continue;
        }

        if (useWorkers and !stopFlag){
            w.downloadPos(mainpos);
            w.searchInfinite();
            workerPing();
            masterSync.notify_one();
            continue;
        }

        workerPing();
        masterSync.notify_one();
    }
}

void Engine::workerPing(){
    std::lock_guard<std::mutex> guard(mute);
    workersReady++;
}

void Engine::createPool(Count nt){
    drainPool();

    clearWorkerNodes();
    //assert(workersReady == workerCount);

    for (Index i = 0; i < nt - 1; i++){
        workerPool.emplace_back(&Engine::runWorker, this, i);
    }

    hasWorkers = nt - 1;
    workerCount = nt - 1;

    masterIdle();
    //assert(workersReady == workerCount);
}

void Engine::drainPool(){
    {
        std::lock_guard<std::mutex> guard(mute);
        //assert(workersReady == workerCount);
        destruct = true;
        pulse++;
    }

    sync.notify_all();

    for (std::thread& t : workerPool){
        t.join();
    }

    workerPool.clear(); //safely clear out vector

    workerCount = 0;
    workersReady = 0;

    destruct = false;
}

template Score Engine::go<true>(Depth, uint64_t, uint64_t, bool);
template Score Engine::go<false>(Depth, uint64_t, uint64_t, bool);
