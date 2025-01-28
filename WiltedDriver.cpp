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

        if (command == "quit"){
            return 0;
        }

        if (command == "isready"){
            std::cout << "readyok\n";
        }

        if (command.substr(0, 17) == "position startpos"){
            engine.setStartPos();

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
            
            std::stringstream extraMoves(command.substr(ms + 6));

            while (!extraMoves.eof()){
                extraMoves >> param;
                engine.sendMove(param);
            }
        }

        if (command.substr(0, 2) == "go"){
            engine.search();
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
        }
    
    }
    

    return 0;
}