// Driver Code for Wilted Engine

#include "Interface.h"

int main(int argc, char* argv[]){
    Engine engine;
    
    Network::loadnet("wilted-net-1-0.bin");

    if ((argc == 2) and (std::string(argv[1]) == "bench")){
        engine.bench();
        return 0;
    }

    Interface proto;
    proto.loop(engine);
    
    return 0;
}