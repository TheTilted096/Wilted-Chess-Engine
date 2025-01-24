/*
Class Definition of Position
Intermediate Object

Contains static evaluation and advanced makeMove

*/

#include "WiltedBitboards.h"

class Move{
    public:
        uint32_t info;

        Move();
        Move(uint32_t&);

        std::string toStr();
        void print();

        int stsq();
        int edsq();
        bool capt();
        int cptp();
        int tpmv();
        bool prmt();
        int tpnd();

        int cstl();
        bool dpsh();
        bool epcp();

        //operator overload OR
        //void operator|=(const uint32_t&);
        //void operator=(const uint32_t&);

        //ton of other things
        
};

class Position : public Bitboards{
    public:
        Move moves[64][96];

        //int scores[2], eScores[2];
        //int mobil[2], eMobil[2];
        //int gamePhase;

        //Bitboard atktbl[2][5];

        //total game phase
        //phases, material values

        //mobilities

        //piece-square tables

        //Bitboard cm;

        Position();

        bool isAttacked(int, bool);
        bool isChecked(bool);
        bool notValid(Move);

        //int evaluate();
        //int evaluateScratch();

        int generateMoves(int);
        //int generateCaptures(int);

        uint64_t perft(int, int);

        void makeRMove(Move);
        void unmakeRMove(Move);

        //void makeMove(Move&);
        //void unmakeMove(Move&); //figure out overloading?


};