// Driver Code for Wilted Engine

#include "Performer.h"
#include "Searcher.h"

int main(int argc, char* argv[]){
    Searcher master;

    if ((argc == 2) and (std::string(argv[1]) == "bench")){
        master.bench();
        return 0;
    }

    std::string versionStr = "Wilted 0.3.1.0";

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
            std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            //std::cout << "option name Hash type spin default 32 min 32 max 32\n";
            std::cout << "option name UCI_Chess960 type check default false\n";
            std::cout << "uciok" << std::endl;
        }

        if (command == "isready"){
            std::cout << "readyok" << std::endl;
        }

        if (command == "ucinewgame"){
            master.newGame();
        }

        if (command.substr(0, 17) == "position startpos"){
            master.pos.setStartPos();

            int movesStart = command.find("moves");
            if (movesStart == 18){ //if there is a moves xx string

                std::stringstream extras(command.substr(movesStart + 6));

                Move given;

                while (!extras.eof()){
                    extras >> param;
                    given = master.gen.unalgebraic(param);
                    if (!given.bad()){
                        master.pos.makeMove(given);
                    }
                    if (given.resets()){
                        master.pos.forget();
                    }
                }
            }
        }

        if (command.substr(0, 12) == "position fen"){
            int movesStart = command.find("moves"); //should return -1 (ULL) if not found

            master.pos.readFen(command.substr(13, movesStart - 13));

            if (movesStart != -1){

                std::stringstream extras(command.substr(movesStart + 6));

                Move given;

                while (!extras.eof()){
                    extras >> param;
                    given = master.gen.unalgebraic(param);
                    if (!given.bad()){
                        master.pos.makeMove(given);
                    }
                    if (given.resets()){
                        master.pos.forget();
                    }
                }
            }
        }

        if (command == "setoption name UCI_Chess960 value true"){ master.pos.setFRC(); }
        if (command == "setoption name UCI_Chess960 value false"){ master.pos.stopFRC(); }

        if (command.substr(0, 2) == "go"){
            std::stringstream searchLimits(command);

            std::string ot = master.pos.toMove ? "wtime" : "btime";
            std::string oi = master.pos.toMove ? "winc" : "binc";

            uint32_t thinkBase = ~0U;
            uint32_t thinkrement = 0U; // increment 0 unless specified
            Depth thinkDepth = Searcher::MAX_PLY;
            uint64_t thinkNodes = ~0ULL;

            while (!searchLimits.eof()){
                searchLimits >> param;
                if (param == ot){ searchLimits >> param; thinkBase = stoi(param); }
                if (param == oi){ searchLimits >> param; thinkrement = stoi(param); }
                if (param == "depth"){ searchLimits >> param; thinkDepth = stoi(param); }
                if (param == "nodes"){ searchLimits >> param; thinkNodes = stoi(param); }
            }

            master.tim.setBounds(thinkBase, thinkrement);
            master.search(thinkDepth, thinkNodes, true);
        }

        if (command.substr(0, 5) == "perft"){
            Depth d = std::stoi(command.substr(6));

            Performer perf(&master.pos);

            perf.run(d);
        }

        // Custom Commands

        if (command.substr(0, 10) == "perftsuite"){
            //got lazy
        }

        if (command == "printpieces"){
            master.pos.print();
        }

        if (command == "printzobrist"){
            master.pos.showZobrist();
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