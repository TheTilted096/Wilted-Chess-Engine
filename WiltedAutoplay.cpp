/*

Autoplay Program for the Wilted Engine

TheTilted096, 2-19-25

*/

#include "WiltedEngine.cpp"

#include <mutex>
#include <thread>

class Game{
    public: 

    Engine players[2];
    std::string* positions;
    Move* moves;
    int16_t* scores;
    uint64_t* reportPack;

    std::string reason;

    int clock;
    bool turn;

    float result;

    std::string opening;

    void play();
    void report(int);

    void feedEngine(const std::string&);

    bool adjudicate();
    void blockRep(int);
    bool getRep(int);
    //bool reportable();

    Game();
    ~Game();

    static void shuffleData();

    static uint64_t maxNodes; //maximum nodes to search
    static int beginReport, endReport, maxHalf; //moves after opening position, moves from end, maximum half move count
    //static bool quiets; // report positions no checks and captures
    static std::ofstream outFile, hOutFile;
    static std::string filename;

    static bool dualReport;
};

uint64_t Game::maxNodes;

int Game::beginReport;
int Game::endReport;
int Game::maxHalf;
bool Game::dualReport;

/*
bool Game::quiets;
*/

std::ofstream Game::outFile;
std::ofstream Game::hOutFile;
std::string Game::filename;

Game::Game(){
    positions = new std::string[512];
    moves = new Move[512];
    scores = new int16_t[512];
    reportPack = new uint64_t[8];

    for (int i = 0; i < 8; i++){
        reportPack[i] = 0ULL;
    }

    dualReport = false;
}

Game::~Game(){
    delete[] positions;
    delete[] moves;
    delete[] scores;
    delete[] reportPack;
}

void Game::feedEngine(const std::string& s){
    int ms = s.find("moves");

    players[turn].readFen(s.substr(13, ms - 13));
    players[turn].beginZobrist();
    std::stringstream em(s.substr(ms + 6));
    std::string t;
    while (!em.eof()){
        em >> t;
        players[turn].sendMove(t);
    }
}

bool Game::adjudicate(){
    if (players[turn].countLegal() == 0){ 
        if (players[turn].isChecked(turn)){ // 'turn' is mated
            result = !turn;
            reason = turn ? "Black" : "White";
            reason += " Mates";
        } else {
            reason = "Stalemate";
            result = 0.5;
        }
    
        return true;
    }

    if (players[turn].countReps(0) > 2){
        result = 0.5;
        reason = "Threefold Repetition";
        return true;
    }

    if (players[turn].insufficient()){
        result = 0.5;
        reason = "Insufficient Material";
        return true;
    }

    if (players[turn].halfMoves() >= 100){ // 50 move rule
        result = 0.5;
        reason = "Fifty Moves Rule";
        return true;
    }
    if (clock > 510){
        result = 0.5;
        reason = "Game Length Adjudication";
        return true;
    }    
    return false;
}

void Game::blockRep(int c){
    reportPack[c >> 6] |= (1ULL << (c & 63));
}

bool Game::getRep(int c){
    return (reportPack[c >> 6] >> (c & 63)) & 1;
}

void Game::play(){
    { //small thing that finds the first side to move
    int i;
    for (i = 0; opening[i] != ' '; i++){}
    turn = opening[i + 1] & 1;
    }

    clock = 0;

    std::string inuci = "position fen " + opening;

    players[0].newGame();
    players[1].newGame();

    //players[0].timeMan(0xFFFFFFFFU, 0); //set timekept = false
    //players[1].timeMan(0xFFFFFFFFU, 0);

    while (true){
        feedEngine(inuci);

        //std::cout << inuci << "\n\n";

        positions[clock] = players[turn].makeFen();  
        
        if (players[turn].halfMoves() > maxHalf){ // Filter out maximum half move count
            blockRep(clock);
        }
        if (players[turn].isChecked(turn)){ // Filter out positions where the side to move is checked
            blockRep(clock);
        }

        if (adjudicate()){ //game ends
            break;
        }

        scores[clock] = players[turn].search(63, maxNodes, false) * (2 * turn - 1);
        moves[clock] = players[turn].getMove();

        if (moves[clock].gcptp()){ // Filter out positions where a capture was made (noisy)
            blockRep(clock);
        }
        if (moves[clock].gtpnd()){ //and promotions
            blockRep(clock);
        }

        if (clock == 0){
            inuci += " moves";
        }
        inuci += ' ';
        inuci += moves[clock].toStr();

        clock++;
        turn ^= 1;
    }
}

void Game::report(int id){
    hOutFile << "GAME " << id << '\n';
    hOutFile << "RESULT: " << result << '\n';
    hOutFile << "REASON: " << reason << '\n';
    hOutFile << "START: " << positions[0] << '\n';
    for (int i = beginReport; i <= clock - endReport; i++){
        if (!getRep(i)){
            hOutFile << "D: " << positions[i] << " | " << scores[i] << " | " << result << '\n';
            outFile << positions[i] << " | " << scores[i] << " | " << result << '\n';
        }
    }
    hOutFile << "FINAL: " << positions[clock] << '\n';
}

/*
void Game::shuffleData(){
    outFile.close(); //When shuffling data, cease editing of stored data.

    std::ifstream firstFile(filename); //Begin Reading

    std::vector<uint32_t> idxs;

    std::string fl;
    uint32_t spin, k = 0U;

    std::getline(firstFile, fl);

    do {
        if (fl.substr(0, 2) == "D:"){
            //std::cout << "Pushed: " << fl << '\n';
            idxs.push_back(k);
        }
        k++;
        std::getline(firstFile, fl);
    } while (!firstFile.eof()); // or fl != ""


    std::ofstream endFile(filename.substr(6));

    std::mt19937_64 wheel(0xCAFEBABE);

    while (!idxs.empty()){
        firstFile.clear();
        firstFile.seekg(0);

        spin = wheel() % idxs.size();

        for (int i = 0; i <= idxs[spin]; i++){ std::getline(firstFile, fl); }

        endFile << fl.substr(3) << '\n';

        idxs.erase(idxs.begin() + spin);
    }

    firstFile.close();
    endFile.close();
}
*/

class Match{
    public: 
    
    int gameLimit, gamesPlayed;
    int numThreads;
    std::thread* workers;

    std::string* book;

    static std::mutex gameBlock;

    Match(int, int); // Num Threads, Num Games

    void getBook();
    void playing(int); //Thread ID

    void run();
    
    ~Match();

    static int openDepth, verifDepth, openScore;
};

int Match::openDepth;
int Match::verifDepth;
int Match::openScore;

std::mutex Match::gameBlock;

Match::Match(int nt, int ng){
    gameLimit = ng;
    gamesPlayed = 0;

    numThreads = nt;
    workers = new std::thread[numThreads];

    getBook();
}

Match::~Match(){
    delete[] workers;
    delete[] book;
}

void Match::getBook(){
    book = new std::string[gameLimit];

    Engine e;
    int oev;

    for (int i = 0; i < gameLimit; i++){
        oev = openScore + 1;
        while (abs(oev) > openScore){
            e.newGame();

            int j = 0, mi;
            while (j < openDepth){
                mi = rand() % e.generateMoves(0);
                e.makeMove<false>(e.moves[0][mi]);
                if (e.notValid(e.moves[0][mi])){
                    e.unmakeMove<false>(e.moves[0][mi]);
                    continue;
                }
                j++;
            }

            oev = e.search(verifDepth, ~0ULL, false);
        }

        book[i] = e.makeFen();
    }    
}

void Match::playing(int threadID){
    Game* gameptr = nullptr;
    int gameNum;

    while (true){
        gameBlock.lock();
        if (gamesPlayed == gameLimit){
            gameBlock.unlock();
            delete gameptr;
            break;
        } else {
            delete gameptr;

            gameptr = new Game();
            gameptr->opening = book[gamesPlayed];

            gameNum = gamesPlayed;
            std::cout << "(Thread " << threadID << ") Started Game " << gameNum << '\n';

            gamesPlayed++;
        }

        gameBlock.unlock();

        gameptr->play();

        gameBlock.lock();
        gameptr->report(gameNum);
        
        std::cout << "(Thread " << threadID << ") Finished Game " << gameNum 
                << "  (" << gameptr->result << ", " << gameptr->reason << ")\n";

        gameBlock.unlock();
    }
}

void Match::run(){
    for (int k = 0; k < numThreads; k++){

        workers[k] = std::thread(&Match::playing, this, k);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int l = 0; l < numThreads; l++){
        workers[l].join();
    }
}

int main(int argc, char* argv[]){
    if (argc != 5){ //file name, num threads, num games
        std::cout << "Correct Usage: ./wauto fileName numThreads numGames numNodes\n";
        return 0;
    }

    srand(time(0));
    Bitboards::initSlideAttacks();

    Game::maxNodes = std::stoi(std::string(argv[4]));

    Game::beginReport = 20;
    Game::endReport = 20;
    Game::maxHalf = 20;

    std::string myOut = argv[1];

    Game::dualReport = true;

    remove(myOut.c_str());
    remove(("H-" + myOut).c_str());

    Game::outFile.open(myOut);
    Game::hOutFile.open("H-" + myOut);
    Game::filename = myOut;

    Match::openDepth = 6;
    Match::verifDepth = 4;
    Match::openScore = 100;

    int threadCount = std::stoi(std::string(argv[2]));
    int gameCount = std::stoi(std::string(argv[3]));

    Match m(threadCount, gameCount);

    m.run();

    /*
    std::cout << "Reprocessing Data...\n";
    Game::shuffleData();
    std::cout << "Done.";
    */

    return 0;
}






















