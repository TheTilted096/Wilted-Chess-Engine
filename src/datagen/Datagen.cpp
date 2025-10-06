// Driver Code for Wilted Datagen

//#include <filesystem>

#include "Match.h"

void bulletizeFile(std::string);

int main(int argc, char* argv[]){
    //bulletizeFile(std::string(argv[1]));

    // #games, #workers, #nodes
    if (argc != 4){
        std::cout << "Correct Usage: " << "./WiltedGen-2-0-0 numGames numThreads numNodes\n";
        return 0;
    }

    Match m(std::stoi(std::string(argv[1])), std::stoi(std::string(argv[2])));
    m.getBook(10, 8, 150);

    Game::maxSoftNodes = std::stoi(std::string(argv[3]));
    Game::openOffset = 10;
    Game::endOffset = 10;
    Game::maxHalf = 20;

    std::string fn = [&](){
        std::string ff = "";
    
        std::time_t currentTime = std::time(0); 
        std::tm* localTime = std::localtime(&currentTime);

        if (localTime->tm_mon < 9){ ff += '0'; }

        ff += std::to_string(localTime->tm_mon + 1);

        if (localTime->tm_mday < 10){ ff += '0'; }
        ff += std::to_string(localTime->tm_mday);

        ff += std::to_string(localTime->tm_year - 100);

        ff += '-';

        ff += std::to_string((int)getMostBit(Game::maxSoftNodes));
        ff += '-';

        ff += std::to_string(m.gameLimit);

        return ff;
    }();

    Game::ofile.open(fn + ".txt");
    Game::hofile.open(fn + "-HR.txt");

    m.run();

    // Convert all to bulletformat

    Game::ofile.close();
    Game::hofile.close();

    bulletizeFile(fn);

    return 0;
}

void bulletizeFile(std::string fname){
    std::cout << "Converting to BulletFormat...\n";
    Position pp;
    pp.setFRC();

    std::ifstream suite(fname + ".txt");
    std::string pt, sg;
    Score i;
    float r;

    int kk = 0;

    std::ofstream bd(fname + ".bin", std::ios::binary);

    while (std::getline(suite, pt)){
        std::stringstream ss(pt);
        std::getline(ss, sg, '|');

        pp.readFen(sg);

        std::getline(ss, sg, '|');
        i = std::stoi(sg);

        std::getline(ss, sg, '|');
        r = static_cast<uint8_t>(std::stof(sg) * 2.0);

        Bullet x = pp.makeBullet(i, r);

        bd.write(reinterpret_cast<const char*>(&x), sizeof(x));
        
        /*
        std::cout << x.occ << '\n';
        for (int xx = 0; xx < std::popcount(x.occ); xx++){
            std::cout << (int)((x.pcs[xx >> 1] >> ((xx & 1) << 2)) & 0xF) << '\n';
        }

        std::cout << (int16_t)x.score << '\n';
        std::cout << (int)x.result << '\n';
        std::cout << (int)x.ksq << '\n';
        std::cout << (int)x.opp_ksq << '\n';
        */

        kk++;
        if (kk % 1000000 == 0){
            std::cout << kk/1000000 << " million positions converted\n";
        }
    }

    std::cout << kk << " positions converted\n";
}