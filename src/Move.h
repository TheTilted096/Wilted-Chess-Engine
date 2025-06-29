// Move Representation Class

/*
0-5: Start Square
6-11: End Square

12-14: Piece Type Moved
15-17: Captured Type
18-20: End Type

21: 0-0
22: 0-0-0

23: Double Push
24: En Passant
25: Promotion

*/

#include "Helpers.h"

class Move{
    static constexpr std::array<char, 5> pieceNames = {'k', 'q', 'r', 'n', 'b'};
    public:
        uint32_t info;

        Move(){ info = 0U; }
        Move(const uint32_t& m){ info = m; }
        Move(const Move& m){ info = m.info; }
        //Move& operator=(const uint32_t m){ info = m; }

        std::string toString() const{ //set up 960 later
            Square start = from();
            Square end = to();

            std::string result = "";
            result += ((start & 7) + 97);
            result += (8 - (start >> 3)) + 48;

            result += ((end & 7) + 97);
            result += (8 - (end >> 3)) + 48;

            if (promoted()){
                result += pieceNames[ending()];
            }

            return result;
        }

        bool bad() const{ return info == Invalid; }

        Square from() const{ return static_cast<Square>(info & 63U); }
        void setFrom(const Square& s){ info = s; } //Move is cleared upon generation of start square

        Square to() const{ return static_cast<Square>((info >> 6) & 63U); }
        void setTo(const Square& s){ info |= (s << 6); }

        Piece moving() const{ return static_cast<Piece>((info >> 12) & 7U); }
        void setMoving(const Piece& p){ info |= (p << 12); }

        Piece captured() const{ return static_cast<Piece>((info >> 15) & 7U); }
        void setCaptured(const Piece& p){ info |= (p << 15); }

        Piece ending() const{ return static_cast<Piece>((info >> 18) & 7U); }
        void setEnding(const Piece& p){ info |= (p << 18); }

        uint8_t castling() const{ return ((info >> 21) & 3U); }

        bool kingCastle() const{ return ((info >> 21) & 1U); }
        void setKingCastle(){ info ^= (1U << 21); }

        bool queenCastle() const{ return ((info >> 22) & 1U); }
        void setQueenCastle(){ info ^= (1U << 22); }

        bool doublePushed() const{ return ((info >> 23) & 1U); }
        void setDoublePush(){ info ^= (1U << 23); }

        bool epCapture() const{ return ((info >> 24) & 1U); }
        void setEpCaps(){ info ^= (1U << 24); }    
        
        bool promoted() const{ return (info >> 25); }
        void setPromote(){ info ^= (1U << 25); }

        bool operator==(const Move& m){
            return info == m.info;
        }
  
        static constexpr uint32_t Invalid = 0xFFFU; //How to make this into a static Move object?
};
