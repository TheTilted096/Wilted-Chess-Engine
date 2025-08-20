// Class Definition for Game Object

#include "../Interface.h"

class Game{
    public:
        std::array<Engine, 2> players;
        
        std::vector<std::string> positions;
        std::vector<Score> scores;

        std::string verdict;

        uint32_t clock;
        Color turn;

        float result;

        Interface host;

        void play(std::string);
        void report(uint32_t);

        bool adjudicate();

        static uint64_t maxSoftNodes;
        static uint32_t openOffset, endOffset, maxHalf;
        static std::ofstream ofile, hofile;
};
