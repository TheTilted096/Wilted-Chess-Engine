// Class Definition for Princes

#include "Move.h"

class Princes{
    public:
        Table<Move, MAX_PLY + 1, MAX_PLY + 1> vars;
        std::array<Index, MAX_PLY + 1> lens;

        Princes(){ clearAll(); }

        void clearLine(Index k){ lens[k] = 0; }

        void clearAll(){ lens.fill(0); }

        void write(const Move& m, Index ply){
           vars[ply][0] = m;
           std::copy_n(vars[ply + 1].begin(), lens[ply + 1], vars[ply].begin() + 1);
           lens[ply] = lens[ply + 1] + 1;                       
        }
};