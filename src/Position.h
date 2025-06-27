//Board Representation Class

#include "Moment.h"
#include "Move.h"

class Position{
    public: //adjust access specifiers later
        Index clock; //half move counter
        std::array<Moment, 256> moments;
        std::array<Move, 256> plays;

        Position();

        Moment& thisMoment(){ return moments[clock]; }

        //Can call Moment functions on current Moment
        //Piece pieceAt(const Square& s) const{ return moments[clock].pieceAt(s); }
        bool insufficient() const{ return moments[clock].insufficient(); }
        Color stm() const{ return moments[clock].toMove; }
        Count sinceReset() const{ return moments[clock].sinceReset(); }
        void empty(){
            clock = 0;
            moments[clock].empty();
        }
        void setStartPos(){
            clock = 0;
            moments[clock].setStartPos();
        }
        void print() const{
            moments[clock].print();
            std::cout << "Clock: " << static_cast<int>(clock) << '\n';
            std::cout << getFen() << "\n\n";
        }
        void beginZobrist(){ moments[clock].beginZobrist(); }

        void forget(){
            moments[0] = moments[clock];
            clock = 0;
        }

        Move lastPlayed() const{ return plays[clock]; }

        //void showZobrist() const;
        //Count countReps(Index) const;

        void readFen(std::string fen){
            clock = 0;
            moments[clock].readFen(fen);
        }

        std::string getFen() const{
            return moments[clock].makeFen();
        }

        void makeMove(const Move& m);
        void unmakeMove();

        //void passMove();
        //void unpassMove();

        //Castling Stuff [Perhaps moved to Generator class]
        
        std::array<Square, 2> kingRookFrom = {h8, h1};
        std::array<Square, 2> queenRookFrom = {a8, a1};

        static constexpr std::array<Square, 2> kingRookTo = {f8, f1};
        static constexpr std::array<Square, 2> queenRookTo = {d8, d1};

        static constexpr std::array<Square, 2> kingKingTo = {g8, g1};
        static constexpr std::array<Square, 2> queenKingTo = {c8, c1};

        std::array<Bitboard, 2> kingSafeMask = {0x70ULL, 0x70ULL << 56};
        std::array<Bitboard, 2> queenSafeMask = {0x1CULL, 0x1CULL << 56};

        std::array<Bitboard, 2> kingOccMask = {0x60ULL, 0x60ULL << 56};
        std::array<Bitboard, 2> queenOccMask = {0xEULL, 0xEULL << 56};

        std::array<uint8_t, 64> rightsChange = 
        {8, 0, 0, 0, 12, 0, 0, 4,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 3, 0, 0, 1};

        bool isFRC = false;

        // FEN parsing

        std::array<std::string, 16> castleStrings = 
        {"-", "K", "Q", "KQ",
        "k", "Kk", "Qk", "KQk",
        "q", "Kq", "Qq", "KQq",
        "kq", "Kkq", "Qkq", "KQkq"};
};
