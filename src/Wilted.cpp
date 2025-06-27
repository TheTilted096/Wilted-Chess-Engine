// Driver Code for Wilted Engine

#include "Performer.h"

int main(int argc, char* argv[]){
    Position engine;
    Generator g;
    g.assign(&engine);

    std::cout << "Wilted 0.0.0.0 by TheTilted096\n";

    std::string command, param;

    while (true){
        getline(std::cin, command);

        if (command == "quit"){
            return 0;
        }

        if (command == "uci"){
            std::cout << "id name Wilted-0-0-0-0\n";
            std::cout << "id author TheTilted096\n";
            //std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            //std::cout << "option name Hash type spin default 32 min 32 max 32\n";
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
                    }
                }
            }
        }

        if (command.substr(0, 5) == "perft"){
            Depth d = std::stoi(command.substr(6));

            Performer perf(&engine);

            perf.run(d);
        }

        if (command == "prints"){
            engine.print();
        }

        if (command == "test"){
            //g.assign(&engine);

            engine.print();

            std::cout << (int) g.countLegal() << '\n';            
            
            engine.print();
        }




    }





    return 0;
}