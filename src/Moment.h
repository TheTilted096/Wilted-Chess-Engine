// Class Definition for Moment

#include "Zobrist.h"

class Moment{
    public:
        std::array<Bitboard, 2> sides;
        std::array<Bitboard, 6> pieces;
        Color toMove;

        uint8_t castleRights;
        Square enPassant;

        Count halfMoves;
        Hash hash;
    
        Moment();
        Moment(const Moment&);

        Piece pieceAt(const Square&) const;

        bool insufficient() const;

        //Color stm() const{ return toMove; }
        Count sinceReset() const{ return halfMoves; }

        Bitboard those(const Color& c, const Piece& p) const{ return sides[c] & pieces[p]; }
        Bitboard our(const Piece& p) const{ return sides[toMove] & pieces[p]; }
        Bitboard their(const Piece& p) const{ return sides[!toMove] & pieces[p]; }

        //Bitboard thoses(const Color& c) const{ return sides[c]; }
        Bitboard ours() const{ return sides[toMove]; }
        Bitboard theirs() const{ return sides[!toMove]; }
        Bitboard occupied() const{ return sides[Black] | sides[White]; }

        Bitboard diagonalPieces() const{ return pieces[3] | pieces[1]; }
        Bitboard straightPieces() const{ return pieces[2] | pieces[1]; }

        uint8_t ourRights() const{ return (castleRights >> (2 * !toMove)) & 3; }

        void readFen(std::string);
        std::string makeFen() const;

        void empty();
        void setStartPos();

        void print() const;

        void beginZobrist();

        // FEN parsing
        static constexpr std::array<char, 21> fenChars = 
        {'/', '1', '2', '3', '4', '5', '6', '7', '8',
        'k', 'q', 'r', 'b', 'n', 'p', 'K', 'Q', 'R', 'B', 'N', 'P'};

        std::array<std::string, 16>* castleChars;
        void setCastleChars(std::array<std::string, 16>* ptr){ castleChars = ptr; } 

};