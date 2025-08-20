// Class Definition for Match object

#include "Game.h"

class Match{
    public:
        uint32_t gameLimit, gamesFinished;
        Count workerCount;

        std::vector<std::string> book;

        static std::mutex matchMute;

        Match(uint32_t, Count);

        void getBook(Depth, Depth, Score);
        void run();
        void runner(Count);

        static std::string getFRClayout(uint8_t&, uint8_t&);
        static std::string getDFRClayout();
};