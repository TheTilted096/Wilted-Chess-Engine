// Driver Code for Wilted Engine

#include "Performer.h"
#include "Engine.h"

int main(int argc, char* argv[]){
    Engine engine;
    Position& mainpos = engine.master.pos;
    Generator& maingen = engine.master.gen;

    if ((argc == 2) and (std::string(argv[1]) == "bench")){
        engine.bench();
        return 0;
    }

    std::string versionStr = "Wilted 0.5.0.0";

    std::cout << versionStr << " by TheTilted096\n";

    std::string command, param;

    bool minPrint = false;

    while (true){
        getline(std::cin, command);

        if (command == "quit"){
            return 0;
        }

        if (command == "uci"){
            std::cout << "id name " << versionStr << '\n';
            std::cout << "id author TheTilted096\n";
            //std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            std::cout << "option name Hash type spin default 32 min 1 max 128\n";
            std::cout << "option name UCI_Chess960 type check default false\n";
            std::cout << "option name Minimal type check default false\n";
            std::cout << "uciok" << std::endl;
        }

        if (command == "isready"){
            std::cout << "readyok" << std::endl;
        }

        if (command == "ucinewgame"){
            engine.newGame();
        }

        if (command.substr(0, 17) == "position startpos"){
            mainpos.setStartPos();

            int movesStart = command.find("moves");
            if (movesStart == 18){ //if there is a moves xx string

                std::stringstream extras(command.substr(movesStart + 6));

                Move given;

                while (!extras.eof()){
                    extras >> param;
                    given = maingen.unalgebraic(param);
                    if (!given.bad()){
                        mainpos.makeMove(given);
                    }
                    if (given.resets()){
                        mainpos.forget();
                    }
                }
            }
        }

        if (command.substr(0, 12) == "position fen"){
            int movesStart = command.find("moves"); //should return -1 (ULL) if not found

            mainpos.readFen(command.substr(13, movesStart - 13));

            if (movesStart != -1){

                std::stringstream extras(command.substr(movesStart + 6));

                Move given;

                while (!extras.eof()){
                    extras >> param;
                    given = maingen.unalgebraic(param);
                    if (!given.bad()){
                        mainpos.makeMove(given);
                    }
                    if (given.resets()){
                        mainpos.forget();
                    }
                }
            }
        }

        if (command.substr(0, 9) == "setoption"){
            if (command.substr(15, 12) == "UCI_Chess960"){
                if (command.substr(34) == "true"){ mainpos.setFRC(); }
                if (command.substr(34) == "false"){ mainpos.stopFRC(); }
            }

            if (command.substr(15, 7) == "Minimal"){
                if (command.substr(29) == "true"){ minPrint = true; }
                if (command.substr(29) == "false"){ minPrint = false; }
            }

            if (command.substr(15, 4) == "Hash"){
                std::size_t s = std::stoi(command.substr(26));
                engine.ttable.resize(s);
            }

        }

        if (command.substr(0, 2) == "go"){
            std::stringstream searchLimits(command);

            std::string ot = mainpos.toMove ? "wtime" : "btime";
            std::string oi = mainpos.toMove ? "winc" : "binc";

            uint32_t thinkBase = ~0U;
            uint32_t thinkrement = 0U; // increment 0 unless specified
            Depth thinkDepth = MAX_PLY;
            uint64_t thinkNodes = ~0ULL;

            while (!searchLimits.eof()){
                searchLimits >> param;
                if (param == ot){ searchLimits >> param; thinkBase = stoi(param); }
                if (param == oi){ searchLimits >> param; thinkrement = stoi(param); }
                if (param == "depth"){ searchLimits >> param; thinkDepth = stoi(param); }
                if (param == "nodes"){ searchLimits >> param; thinkNodes = stoi(param); }
            }

            engine.timer.setBounds(thinkBase, thinkrement);
            engine.go<true>(thinkDepth, thinkNodes, minPrint);
        }

        if (command.substr(0, 5) == "perft"){
            Depth d = std::stoi(command.substr(6));

            Performer perf(&mainpos);

            perf.run(d);
        }

        // Custom Commands

        if (command.substr(0, 10) == "perftsuite"){
            //got lazy
        }

        if (command == "printpieces"){
            mainpos.print();
        }

        if (command == "printzobrist"){
            mainpos.showZobrist();
        }

        if (command == "test"){

        }

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