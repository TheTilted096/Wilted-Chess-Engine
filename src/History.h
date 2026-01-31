// Class Definition for History

#include "Move.h"

class History{
    public:
        Table<int16_t, 2, 64, 64> quiet;
        Table<int16_t, 2, 6, 64, 6, 64> counters;

        History();

        void empty();

        void gravity(int16_t&, int);

        void updateQuiet(const Move& m, const Color& c, int t);
        int16_t& quietEntry(const Move& m, const Color& c);

        void updateCounter(const Color&, const Move&, const Move&, int);
        int16_t& counterEntry(const Color&, const Move&, const Move&);
};