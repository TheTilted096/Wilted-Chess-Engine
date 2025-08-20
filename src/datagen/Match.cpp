// Function Definitions for Match Object

#include "Match.h"

Match::Match(uint32_t gl, Count wc){
    workerCount = wc;
    gameLimit = gl;
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
}

