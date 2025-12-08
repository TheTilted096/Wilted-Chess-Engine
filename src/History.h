// Class Definition for History

#include "Move.h"

class History{
    public:
        Table<int16_t, 2, 64, 64> quiet;

        History();

        void empty();
        void updateQuiet(const Move&, const Color&, int);
        int16_t quietEntry(const Move&, const Color&);

};