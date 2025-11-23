// Function Definitions for Match Object

#include "Match.h"

std::mutex Match::matchMute;

Match::Match(uint32_t gl, Count wc){
    workerCount = wc;
    gameLimit = gl;

    gamesFinished = 0;
    bookEntriesCollected = 0;
}

std::string Match::getFRClayout(uint8_t& x, uint8_t& y){
    Hash seed = std::chrono::steady_clock::now().time_since_epoch().count();

    x = (randomize(seed) & 3) << 1; // 0, 2, 4, 6
    y = ((randomize(seed) & 3) << 1) + 1; // 1, 3, 5, 7

    std::string rank = "        ";

    rank[x] = 'B';
    rank[y] = 'B';

    std::vector<uint8_t> rem = {0, 1, 2, 3, 4, 5, 6, 7};

    rem.erase(rem.begin() + y);
    rem.erase(rem.begin() + (x - (x > y)));

    x = randomize(seed) % 6;
    rank[rem[x]] = 'Q';
    rem.erase(rem.begin() + x);

    x = randomize(seed) % 5;
    rank[rem[x]] = 'N';
    rem.erase(rem.begin() + x);
    
    x = randomize(seed) % 4;
    rank[rem[x]] = 'N';
    rem.erase(rem.begin() + x);

    x = rem[0];
    y = rem[2];

    rank[x] = 'R';
    rank[rem[1]] = 'K';
    rank[y] = 'R';

    return rank;
}

std::string Match::getDFRClayout(){
    uint8_t x, y;
    std::string c = "    ";

    std::string r = getFRClayout(x, y);

    c[2] = y + 97;
    c[3] = x + 97;

    std::string f = "";

    for (const char& c : r){
        f += c + 32;
    }

    f += "/pppppppp/8/8/8/8/PPPPPPPP/";
    
    f += getFRClayout(x, y);

    c[0] = y + 65;
    c[1] = x + 65;

    f += " w ";
    f += c;
    f += " - 0 1";

    return f;
}

void Match::getBook(Depth od, Depth vd, Score vs){
    book.clear();
    bookEntriesCollected = 0;

    for (int k = 0; k < workerCount; k++){
        workers.emplace_back(&Match::getBookWorker, this, k, od, vd, vs);
    }

    for (int l = 0; l < workerCount; l++){
        workers[l].join();
    }

    workers.clear();
}

void Match::getBookWorker(Count threadID, Depth od, Depth vd, Score vs){
    Engine e;
    e.mainpos.setFRC();

    MoveList ml;

    Hash seed = std::chrono::steady_clock::now().time_since_epoch().count() ^ ((Hash)threadID << 32);

    Score oev;
    int entryNum;
    std::string fenStr;

    while (true){
        matchMute.lock();

        if (bookEntriesCollected == gameLimit){
            matchMute.unlock();
            break;
        } else {
            entryNum = bookEntriesCollected;
            bookEntriesCollected++;
        }

        matchMute.unlock();

        oev = SCORE_INF;
        while (abs(oev) > vs){
            e.newGame();
            e.mainpos.readFen(getDFRClayout());

            int j = 0, mi;
            while (j < od){
                mi = randomize(seed) % e.maingen.generateMoves(ml);
                e.mainpos.makeMove(ml[mi]);
                if (e.maingen.countLegal() == 0){
                    e.mainpos.unmakeMove();
                    continue;
                }
                j++;
            }

            oev = e.goDepth(vd);
        }

        fenStr = e.mainpos.makeFen();

        matchMute.lock();

        if (entryNum % 1000 == 0){
            std::cout << "(Thread " << (int)threadID << ") " << entryNum << " of " << gameLimit << " openings collected.\n";
        }

        book.push_back(fenStr);

        matchMute.unlock();
    }
}

void Match::runner(Count threadID){
    Game g;
    int gameNum;
    std::string opener;

    while (true){
        matchMute.lock();

        if (gamesFinished == gameLimit){
            matchMute.unlock();
            break;
        } else {
            gameNum = gamesFinished;
            opener = book[gameNum];

            std::cout << "(Thread " << (int)threadID << ") Started Game " << gameNum << '\n';

            gamesFinished++;
        }

        matchMute.unlock();

        g.play(opener);

        matchMute.lock();
        g.report(gameNum);

        std::cout << "(Thread " << (int)threadID << ") Finished Game " << gameNum 
                << "  (" << g.result << ", " << g.verdict << ")\n";

        matchMute.unlock();        
    }
}

void Match::run(){
    for (int k = 0; k < workerCount; k++){
        workers.emplace_back(&Match::runner, this, k);
    }

    for (int l = 0; l < workerCount; l++){
        workers[l].join();
    }
}



