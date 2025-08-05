// Class Definition for Princes

#include "Move.h"

class Princes{
    public:
        static constexpr std::size_t PRINCE_SIZE = (MAX_PLY) * (MAX_PLY + 1) / 2;
        Move vars[PRINCE_SIZE]; //perhaps make this impl less C-style
        Index heights[MAX_PLY + 1];

        Princes(){ clearAll(); }

        Move* line(int k){ //gets pvs at depth k
            return &vars[k * (2 * MAX_PLY + 1 - k) / 2];
        }

        void clearLine(int k){
            heights[k] = 0;
        }

        void clearAll(){
            for (int i = 0; i < MAX_PLY; i++){
                heights[i] = 0;
            }
        }

        void write(const Move& m, Index ply){
            Move* tail = line(ply + 1);
            Move* start = line(ply);
            start[0] = m;
            std::memcpy(start + 1, tail, sizeof(Move) * heights[ply + 1]);
            heights[ply] = heights[ply + 1] + 1;
        }
};