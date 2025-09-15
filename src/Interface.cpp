// Function Definitions for Interface (UCI Handler)

#include "Interface.h"

void Interface::loop(Engine& e){
    std::string versionStr = "Wilted 0.8.2.0";

    std::cout << versionStr << " by TheTilted096\n";

    while (true){
        getline(std::cin, command);

        if (command == "quit"){
            break;
        }

        if (command == "uci"){
            std::cout << "id name " << versionStr << '\n';
            std::cout << "id author TheTilted096\n";
            std::cout << "option name Threads type spin default 1 min 1 max 8\n";
            std::cout << "option name Hash type spin default 32 min 1 max 128\n";
            std::cout << "option name UCI_Chess960 type check default false\n";
            std::cout << "option name Minimal type check default false\n";
            std::cout << "option name Softnodes type check default value false\n";
            std::cout << "uciok" << std::endl;
        }

        if (command == "isready"){
            std::cout << "readyok" << std::endl;
        }

        if (command == "ucinewgame"){
            e.newGame();
        }

        if (command.substr(0, 17) == "position startpos"){
            positionStartpos(e);
        }

        if (command.substr(0, 12) == "position fen"){
            positionFen(e);
        }

        if (command.substr(0, 9) == "setoption"){
            setoption(e);
        }
        
        if (command.substr(0, 2) == "go"){
            std::stringstream searchLimits(command);

            std::string ot = e.mainpos.toMove ? "wtime" : "btime";
            std::string oi = e.mainpos.toMove ? "winc" : "binc";

            uint32_t thinkBase = ~0U;
            uint32_t thinkrement = 0U; // increment 0 unless specified
            Depth thinkDepth = MAX_PLY;
            uint64_t thinkNodes = ~0ULL;
            uint64_t softThinkNodes = ~0ULL;

            while (!searchLimits.eof()){
                searchLimits >> param;
                if (param == ot){ searchLimits >> param; thinkBase = stoi(param); }
                if (param == oi){ searchLimits >> param; thinkrement = stoi(param); }
                if (param == "depth"){ searchLimits >> param; thinkDepth = stoi(param); }
                if (param == "nodes"){ 
                    searchLimits >> param; 
                    if (asSoftNodes){
                        softThinkNodes = stoi(param);
                    } else {
                        thinkNodes = stoi(param); 
                    }
                }
            }

            e.timer.setBounds(thinkBase, thinkrement);
            e.go<true>(thinkDepth, thinkNodes, softThinkNodes, minPrint);            
        }

        if (command.substr(0, 5) == "perft"){
            Depth d = std::stoi(command.substr(6));
            Performer perf(&e.mainpos);
            perf.run(d);
        }

        if (command.substr(0, 6) == "access"){ // custom commands
            param = command.substr(7);
            if (param == "printpieces"){
                e.mainpos.print();
            }
            if (param == "printzobrist"){
                e.mainpos.showZobrist();
            }
            if (param.substr(0, 10) == "perftsuite"){
                param = command.substr(18);
                Position pp;

                int f = param.find("fischer");

                std::ifstream suite(param);

                if (f != -1){
                    std::cout << "FRC Suite Detected\n";
                    pp.setFRC();
                }

                uint32_t l = 0;
                Depth d;
                uint64_t tnc, enc;
                std::string fl;

                Performer perf(&pp);

                uint64_t lifetime = 0ULL;

                auto start = std::chrono::steady_clock::now();

                while (std::getline(suite, param)){
                    f = param.find(';');
                    fl = param.substr(0, f);
                    pp.readFen(fl);
                    std::istringstream sline(param.substr(f));
                    
                    //d = 1;

                    while ((sline >> param)){
                        d = std::stoi(param.substr(2));
                        sline >> param;
                        tnc = std::stoull(param); // string to u64, i think
                        
                        enc = perf.perft<false>(d);    
                        
                        if (enc != tnc){
                            std::cout << fl << '\n';
                            std::cout << "Depth: " << (int)d << "   Target: " << tnc << "   Obtained: " << enc << '\n';
                            exit(1);
                        }

                        lifetime += enc;

                        //std::cout << "Depth " << (int)d << " complete\n";
                        //d++;
                    }

                    l++;

                    if (l % 10 == 0){
                        std::cout << l << " positions complete.\n";
                    }
                }

                auto end = std::chrono::steady_clock::now();
                double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                int64_t nps = (lifetime / dur) * 1000000;
                std::cout << nps << " nps\n\n";
            }
            if (param == "randompsqt"){ // generates randomized psqt
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
    }
}

void Interface::positionStartpos(Engine& e){
    e.mainpos.setStartPos();

    int movesStart = command.find("moves");
    if (movesStart == 18){ //if there is a moves xx string

        std::stringstream extras(command.substr(movesStart + 6));

        Move given;

        while (extras >> param){
            //std::cout << "PART:" << param << '\n';
            given = e.maingen.unalgebraic(param);
            if (!given.bad()){
                e.mainpos.makeMove(given);
            }
            if (given.resets()){
                e.mainpos.forget();
            }
        }
    }
}

void Interface::positionFen(Engine& e){
    int movesStart = command.find("moves"); //should return -1 (ULL) if not found

    e.mainpos.readFen(command.substr(13, movesStart - 13));

    if (movesStart != -1){

        std::stringstream extras(command.substr(movesStart + 6));

        Move given;

        while (extras >> param){
            given = e.maingen.unalgebraic(param);
            if (!given.bad()){
                e.mainpos.makeMove(given);
            }
            if (given.resets()){
                e.mainpos.forget();
            }
        }
    }
}

void Interface::setoption(Engine& e){
    if (command.substr(15, 12) == "UCI_Chess960"){
        if (command.substr(34) == "true"){ e.mainpos.setFRC(); }
        if (command.substr(34) == "false"){ e.mainpos.stopFRC(); }
    }

    if (command.substr(15, 7) == "Minimal"){
        if (command.substr(29) == "true"){ minPrint = true; }
        if (command.substr(29) == "false"){ minPrint = false; }
    }

    if (command.substr(15, 9) == "Softnodes"){
        if (command.substr(31) == "true"){ asSoftNodes = true; }
        if (command.substr(31) == "false"){ asSoftNodes = false; }
    }

    if (command.substr(15, 4) == "Hash"){
        std::size_t s = std::stoi(command.substr(26));
        e.ttable.resize(s);
    }

    if (command.substr(15, 7) == "Threads"){
        std::size_t s = std::stoi(command.substr(29));
        e.createPool(s);
    }
}







