/*
Main Driver Code for Wilted Engine

TheTilted096
12-19-24

*/

#include "WiltedEngine.cpp"

int main(int argc, char* argv[]){
    srand(time(0));
    Bitboards::initSlideAttacks();

    Engine engine;

    std::cout << "Wilted by TheTilted096\n";

    std::string command, param;

    while (true){
        getline(std::cin, command);

        if (command == "uci"){
            std::cout << "id name Wilted-Pre\n";
            std::cout << "id author TheTilted096\n";
            std::cout << "option name Threads type spin default 1 min 1 max 1\n";
            std::cout << "option name Hash type spin default 32 min 32 max 32\n";
            std::cout << "uciok\n";
        }

        if (command == "ucinewgame"){
            engine.newGame();
        }

        if (command == "quit"){
            return 0;
        }

        if (command == "isready"){
            std::cout << "readyok\n";
        }

        if (command.substr(0, 17) == "position startpos"){
            engine.setStartPos();
            engine.beginZobrist();

            if (command.length() > 25){
                std::stringstream extraMoves(command.substr(24));

                while (!extraMoves.eof()){
                    extraMoves >> param;
                    engine.sendMove(param);
                }
            }
        }

        if (command.substr(0, 12) == "position fen"){
            int ms = command.find("moves");

            engine.readFen(command.substr(13, ms - 13));

            engine.beginZobrist();
            
            std::stringstream extraMoves(command.substr(ms + 6));

            while (!extraMoves.eof()){
                extraMoves >> param;
                engine.sendMove(param);
            }
        }

        if (command.substr(0, 2) == "go"){
            std::stringstream srprm(command);

            std::string ourTime = engine.who() ? "wtime" : "btime";
            std::string ourInc = engine.who() ? "winc" : "binc";

            uint32_t ttb = 0xFFFFFFFFU; //assume absurdly large time
            uint32_t tti = 0U; //increment 0 unless specified
            int tdb = 63;
            uint64_t mnb = ~0ULL;

            //bool mtovr = false; //don't support movetime

            while (!srprm.eof()){
                srprm >> param;
                if (param == ourTime){
                    srprm >> param;
                    ttb = stoi(param);
                }
                if (param == ourInc){
                    srprm >> param;
                    tti = stoi(param);
                }
                if (param == "depth"){
                    srprm >> param;
                    tdb = std::min(63, stoi(param)); //cap search depth for some reason
                }
                if (param == "nodes"){
                    srprm >> param;
                    mnb = stoi(param);
                }
                /*
                if (param == "movetime"){
                    mtovr = true;
                    srprm >> param;
                    ttb = stoi(param);
                }
                */
            }

            engine.timeMan(ttb, tti);
            engine.search(tdb, mnb, true);    
        }

        if (command == "printpieces"){
            engine.print();
        }

        if (command.substr(0, 5) == "perft"){
            int d = std::stoi(command.substr(6));
            auto start = std::chrono::steady_clock::now();

            uint64_t pn = engine.perft(d, 0);

            auto end = std::chrono::steady_clock::now();
            double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
            int nps = (pn / dur) * 1000000;
            std::cout << nps << " nps\n\n";
        }
    
        if (command == "testfunction"){
            //maybe fix output for easier pasting idk
            /*
            std::ofstream pst("randpsqts.txt");

            int buf[6] = {40, 90, 50, 35, 30, 10};

            pst << "int body[6][64] = \n{{";
            for (int i = 0; i < 6; i++){
                for (int j = 0; j < 63; j++){
                    pst << (rand() % (2 * buf[i]) - buf[i]) << ", ";
                    if (j % 8 == 7){
                        pst << '\n';
                    }
                }
                pst << (rand() % (2 * buf[i]) - buf[i]) << "},\n\n{";
            }
            */
        
            /* Incremental Eval Test
            engine.evaluateScratch();
            std::cout << "scores[1]: " << engine.scores[1] << '\n';
            std::cout << "scores[0]: " << engine.scores[0] << '\n';

            //engine.sendMove("e2e4");
            //Move proto(9865524);
            engine.makeMove<true>(Move(1188962U));

            std::cout << "make c4b6\n";

            std::cout << "scores[1]: " << engine.scores[1] << '\n';
            std::cout << "scores[0]: " << engine.scores[0] << '\n';

            std::cout << "unmake c4b6\n";

            engine.unmakeMove<true>(Move(1188962U));

            std::cout << "scores[1]: " << engine.scores[1] << '\n';
            std::cout << "scores[0]: " << engine.scores[0] << '\n';
            */
       
            /* Zobrist Key Generation
            std::mt19937_64 twist{0xBF4C588392769BFFULL};

            std::ofstream zfile("randKeys.txt");

            zfile << std::hex;
            zfile << "static constexpr Hash zpk[2][6][64] =\n{";

            //zfile << "0x" << std::uppercase << twist() << "ULL" << std::nouppercase << ", ";
            for (int i = 0; i < 2; i++){
                zfile << '{';
                for (int j = 0; j < 6; j++){
                    zfile << '{';
                    for (int k = 0; k < 64; k++){
                        zfile << "0x" << std::uppercase << twist() << "ULL" << std::nouppercase << ", ";
                        if (k % 4 == 3){
                            zfile << '\n';
                        }
                    }
                    zfile << "},\n\n";
                }
                zfile << '}';
            }

            zfile << "\n\nstatic constexpr Hash zck[16] = \n{";

            for (int i = 0; i < 16; i++){
                zfile << "0x" << std::uppercase << twist() << "ULL" << std::nouppercase << ", ";
                if (i % 4 == 3){
                    zfile << '\n';
                }
            }

            zfile << "\n\nstatic constexpr Hash zek[8] = \n{";
            for (int i = 0; i < 8; i++){
                zfile << "0x" << std::uppercase << twist() << "ULL" << std::nouppercase << ", ";
                if (i % 4 == 3){
                    zfile << '\n';
                }
            }

            zfile << "\n\nstatic constexpr Hash ztk = ";
            zfile << "0x" << std::uppercase << twist() << "ULL" << std::nouppercase << ";";
            */
       
            /*
            engine.showZobrist();   

            //engine.quiesce(-30000, 30000, 0);
            //engine.print();

            
            int b = engine.generateMoves(0);
            engine.scoreMoves(0, b, 0);
            engine.sortMoves(0, b);

            for (int i = 0; i < b; i++){
                std::cout << engine.moves[0][i].toStr() << ": " << engine.mprior[0][i] << '\n';
            }
            */

            //std::cout << engine.countLegal() << '\n';
            /*
            engine.evaluateScratch();

            std::cout << engine.scores[1] << ", " << engine.scores[0] << '\n';
            std::cout << engine.eScores[1] << ", " << engine.eScores[0] << '\n';
            std::cout << engine.gamePhase << '\n';

            
            //engine.makeMove<true>(8439149); //Qxf6
            //engine.makeMove<true>(134384054); //g4
            //engine.makeMove<true>(71306940); //0-0
            engine.makeMove<true>(17203208);

            std::cout << engine.scores[1] << ", " << engine.scores[0] << '\n';
            std::cout << engine.eScores[1] << ", " << engine.eScores[0] << '\n';
            std::cout << engine.gamePhase << '\n';

            //engine.unmakeMove<true>(134384054); //g4
            //engine.unmakeMove<true>(71306940);
            //engine.unmakeMove<true>(8439149);

            engine.unmakeMove<true>(17203208);

            std::cout << engine.scores[1] << ", " << engine.scores[0] << '\n';
            std::cout << engine.eScores[1] << ", " << engine.eScores[0] << '\n';
            std::cout << engine.gamePhase << '\n';
            */
            
            /*
            int k = engine.generateMoves(0);
            for (int i = 0; i < k; i++){
                std::cout << engine.moves[0][i].toStr() << ": " << engine.moves[0][i].info << '\n';
            }
            */





           
        }   
    }
    

    return 0;
}