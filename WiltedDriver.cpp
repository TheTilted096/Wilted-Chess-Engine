/*
Main Driver Code for Wilted Engine

TheTilted096
12-19-24

*/

#include "WiltedPosition.h"

int main(int argc, char* argv[]){
    Bitboards::initSlideAttacks();

    Position p;

    std::cout << "Wilted by TheTilted096\n";

    std::string command;

    while (true){
        getline(std::cin, command);

        if (command == "quit"){
            return 0;
        }

        if (command.substr(0, 17) == "position startpos"){
            p.setStartPos();
        }

        if (command.substr(0, 12) == "position fen"){
            p.readFen(command.substr(13));
        }

        if (command == "printpieces"){
            p.print();
        }

        if (command.substr(0, 5) == "perft"){
            int d = std::stoi(command.substr(6));
            auto start = std::chrono::steady_clock::now();

            uint64_t pn = p.perft(d, 0);

            auto end = std::chrono::steady_clock::now();
            auto dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
            int nps = pn / ((double) dur) * 1000000;
            std::cout << nps << " nps\n\n";
        }
    }
    

    return 0;
}