// Class Definition for History

#include "Move.h"

class History{
    public:
        Table<int16_t, 2, 64, 64> quiet;
        Table<int16_t, 2, 6, 6, 64> noisy;

        History();

        void empty();
        void updateQuiet(const Move&, const Color&, int);
        int16_t quietEntry(const Move&, const Color&);

        void updateNoisy(const Move&, const Color&, int);
        int16_t noisyEntry(const Move&, const Color&);

};