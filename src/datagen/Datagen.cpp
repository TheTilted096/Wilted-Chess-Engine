// Driver Code for Wilted Datagen

//#include <filesystem>

#include "Match.h"

int main(int argc, char* argv[]){

    // #games, #workers, #nodes

    if (argc < 3){
        std::cout << "Correct Usage: " << "./WiltedGen-1-0-0 numGames numThreads numNodes\n";
        return 0;
    }

    Match m(std::stoi(std::string(argv[1])), std::stoi(std::string(argv[2])));
    m.getBook(6, 6, 100);

    Game::maxSoftNodes = std::stoi(std::string(argv[3]));
    Game::openOffset = 0;
    Game::endOffset = 0;
    Game::maxHalf = 100;

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

    //std::filesystem::create_directory("data");

    Game::ofile.open(fn + ".txt");
    Game::hofile.open(fn + "-HR.txt");

    m.run();

    return 0;
}