// Position Class Definition

#pragma once

#include "Attacks.h"
#include "Move.h"
#include "Zobrist.h"

class Position{
    public:
        // Make-Unmake in place
        std::array<Bitboard, 2> sides;
        std::array<Bitboard, 6> pieces;
        Color toMove;

        // Discard in Unmake
        std::array<uint8_t, 256> castleRights; //last 4 bits q k Q K
        std::array<Square, 256> enPassant;

        std::array<Count, 256> halfMoves;
        std::array<Hash, 256> hashes;

        std::array<Move, 256> plays;

        Index clock;

        Position();
        Position(const Position&);

        Piece pieceAt(const Square&) const;
        bool insufficient() const;

        Bitboard isAttacked(const Square&, const Color&) const;
        Bitboard isChecked(const Color&) const;

        //bool illegal() const;
        //bool isLegal(const Move&) const;

        Count sinceReset() const{ return halfMoves[clock]; }

        Bitboard those(const Color& c, const Piece& p) const{ return sides[c] & pieces[p]; }
        Bitboard our(const Piece& p) const{ return sides[toMove] & pieces[p]; }
        Bitboard their(const Piece& p) const{ return sides[!toMove] & pieces[p]; }
        Bitboard any(const Piece& p) const{ return pieces[p]; }

        Bitboard thoses(const Color& c) const{ return sides[c]; }
        Bitboard ours() const{ return sides[toMove]; }
        Bitboard theirs() const{ return sides[!toMove]; }
        Bitboard occupied() const{ return sides[Black] | sides[White]; }

        Bitboard diagonalPieces() const{ return pieces[Bishop] | pieces[Queen]; }
        Bitboard straightPieces() const{ return pieces[Rook] | pieces[Queen]; }

        bool onlyPawns(){ return !(sides[toMove] ^ (sides[toMove] & (pieces[King] | pieces[Pawn]))); }

        uint8_t ourRights() const{ return (castleRights[clock] >> (2 * !toMove)) & 3; }
        uint8_t thoseRights(const Color& c) const{ return (castleRights[clock] >> (2 * !c)) & 3; }
        Square thisPassant() const{ return enPassant[clock]; }
        Hash thisHash() const{ return hashes[clock]; }

        void readFen(std::string);
        std::string makeFen() const;
        Bullet makeBullet(const Score&, const uint8_t&) const;

        void empty();
        void setStartPos();

        void print() const;

        void beginZobrist();
        void showZobrist() const;
        Count repetitions(Index) const;

        void forget();

        Move lastPlayed() const{ return plays[clock]; }

        void makeMove(const Move& m);
        void unmakeMove();

        void passMove();
        void unpassMove();

        std::string moveName(const Move& m, bool toFlip = false) const;

        bool isFRC = false;
        void setFRC(){
            if (!isFRC){
                isFRC = true;
                makeCastleTable({'H', 'A', 'h', 'a'});
            }
        }
        void stopFRC(){
            if (isFRC){
                isFRC = false;
                restoreCastling();
                setStartPos();
            }
        }

        void deduceCastling(std::string);
        void restoreCastling();

        //Castling Stuff
        
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

        // FEN parsing
        static constexpr std::array<char, 21> fenChars = 
        {'/', '1', '2', '3', '4', '5', '6', '7', '8',
        'k', 'q', 'r', 'b', 'n', 'p', 'K', 'Q', 'R', 'B', 'N', 'P'};

        std::array<std::string, 16> castleStrings =
        {"-", "K", "Q", "KQ",
        "k", "Kk", "Qk", "KQk",
        "q", "Kq", "Qq", "KQq",
        "kq", "Kkq", "Qkq", "KQkq"};

        void makeCastleTable(const std::array<char, 4>& parts);

};