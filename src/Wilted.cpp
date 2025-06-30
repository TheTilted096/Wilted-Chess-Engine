// Driver Code for Wilted Engine

#include <fstream>
#include <random>
#include "Performer.h"

int main(int argc, char* argv[]){
    Position engine;
    Generator g;
    g.assign(&engine);

    std::string versionStr = "Wilted 0.0.1.0";

    std::cout << versionStr << " by TheTilted096\n";

    std::string command, param;

    while (true){
        getline(std::cin, command);

        if (command == "quit"){
            return 0;
        }

        if (command == "uci"){
            std::cout << "id name " << versionStr << '\n';
            std::cout << "id author TheTilted096\n";
            //std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            //std::cout << "option name Hash type spin default 32 min 32 max 32\n";
            std::cout << "option name UCI_Chess960 type check default false\n";
            std::cout << "uciok" << std::endl;
        }

        if (command == "isready"){
            std::cout << "readyok" << std::endl;
        }

        if (command.substr(0, 17) == "position startpos"){
            engine.setStartPos();

            int movesStart = command.find("moves");
            if (movesStart == 18){ //if there is a moves xx string
                //std::cout << "Parsing -> " << command.substr(movesStart + 6) << '\n';

                std::stringstream extras(command.substr(movesStart + 6));

                Move given;

                while (!extras.eof()){
                    extras >> param;
                    given = g.unalgebraic(param);
                    if (!given.bad()){
                        engine.makeMove(given);
                    }
                }
            }
        }

        if (command.substr(0, 12) == "position fen"){
            int movesStart = command.find("moves"); //should return -1 (ULL) if not found
            
            //std::string fen = command.substr(13, movesStart - 13);
            //std::cout << "fen:" << fen << '\n';
            engine.readFen(command.substr(13, movesStart - 13));

            //std::cout << "len: " << 13 + fen.length() << '\n';
            //std::cout << movesStart << '\n';

            if (movesStart != -1){
                //std::cout << "Parsing -> " << command.substr(movesStart + 6) << '\n';

                std::stringstream extras(command.substr(movesStart + 6));

                Move given;

                while (!extras.eof()){
                    extras >> param;
                    given = g.unalgebraic(param);
                    if (!given.bad()){
                        engine.makeMove(given);
                        //std::cout << "given info: " << given.info << '\n';
                    }
                }
            }
        }

        if (command == "setoption name UCI_Chess960 value true"){ engine.setFRC(); /*std::cout << "chungus\n";*/ }
        if (command == "setoption name UCI_Chess960 value false"){ engine.stopFRC(); }

        if (command.substr(0, 5) == "perft"){
            Depth d = std::stoi(command.substr(6));

            Performer perf(&engine);

            perf.run(d);
        }

        if (command == "printpieces"){
            engine.print();
        }

        if (command == "printzobrist"){
            engine.showZobrist();
        }

        if (command == "test"){

        }

        // Custom Commands
        // Customized PSQT
        if (command == "randompsqt"){
            srand(time(0));
            std::ofstream spam("spam.txt");
            std::array<Score, 6> bounds = {16, 36, 20, 14, 12, 4};

            for (Score s : bounds){
                spam << '{';
                for (int i = 0; i < 64; i++){
                    spam << (rand() % s) - (s >> 1) << ", ";
                    if ((i & 7) == 7 and (i != 63)){
                        spam << '\n';
                    }
                }
                spam << "}, \n\n";
            }
        }
    
    
    }





    return 0;
}