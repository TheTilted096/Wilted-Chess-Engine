// Function Definitions for Moment

#include "Moment.h"

Moment::Moment(){
    empty();
}

Moment::Moment(const Moment& m){
    sides = m.sides;
    pieces = m.pieces;
    toMove = m.toMove;
    castleRights = m.castleRights;
    halfMoves = m.halfMoves;
    enPassant = m.enPassant;
    hash = m.hash;
}

Piece Moment::pieceAt(const Square& s) const{
    Bitboard sqbb = squareBitboard(s);
    Piece p = King;
    for (const Bitboard& b : pieces){
        if (sqbb & b){ return p; }
        p++;
    }
    return p;
}

bool Moment::insufficient() const{
    if (pieces[Queen] | pieces[Rook] | pieces[Pawn]){
        return false;
    }

    return std::popcount<Bitboard>((pieces[Bishop] | pieces[Knight]) < 2);
}

void Moment::readFen(std::string fen){
    //Example: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1

    empty();

    std::stringstream segments(fen);
    std::string feed;
    segments >> feed; //piece placement segment

    Index ind;
    Square sq = a8;

    for (char cc : feed){
        for (ind = 0; ; ind++){
            if (fenChars[ind] == cc){
                break;
            }
        }

        if (ind < 9){ //Empty Squares
            sq += ind;
            continue;
        }
        if (ind < 15){ //Black Pieces
            pieces[ind - 9] |= squareBitboard(sq);
            sides[Black] |= squareBitboard(sq);
            sq++;
            continue;
        }

        //White Pieces
        pieces[ind - 15] |= squareBitboard(sq);
        sides[White] |= squareBitboard(sq);
        sq++;
    }

    segments >> feed; //side to move
    toMove = static_cast<Color>(feed[0] & 1); //'w' = 119 and 'b' = 98, which have different last bit

    segments >> feed; //castling rights
    for (ind = 0; ; ind++){
        if ((*castleChars)[ind] == feed){
            break;
        }
    }
    castleRights = ind;

    segments >> feed; //EP Square
    enPassant = (feed != "-") ? static_cast<Square>((feed[0] - 97) + 8 * (56 - feed[1])) : XX;

    //Half Move Clock
    segments >> feed;
    halfMoves = std::stoi(feed);    

    beginZobrist();
}

std::string Moment::makeFen() const{
    Bitboard occ= sides[White] | sides[Black];
    std::string result = "";

    Bitboard squarebb;

    Count emptyCount;

    for (Count i = 0; i < 8; i++){ //for each of 8 ranks
        emptyCount = 0;
        for (Count j = 0; j < 8; j++){ //go through each square
            squarebb = squareBitboard(static_cast<Square>(8 * i + j));

            if (squarebb & occ){ //if its occupied
                if (emptyCount != 0){ //unload empty squares
                    result += (emptyCount + 48);
                }

                Piece p = pieceAt(static_cast<Square>(8 * i + j));
                if (squarebb & sides[White]){
                    result += fenChars[p + 15];
                } else {
                    result += fenChars[p + 9];
                }

                emptyCount = 0; // no longer empty;
            } else { //otherwise, count the empty square
                emptyCount++;
            }
        }
        if (emptyCount != 0){ //at the end of a rank, write the empty squares
            result += (emptyCount + 48);
        }

        if (i != 7){ //put '/' between ranks
            result += '/';
        }
    }

    result += ' ';
    result += (toMove ? 'w' : 'b');
    result += ' ';
    result += (*castleChars)[castleRights];
    result += ' ';

    if (enPassant == XX){
        result += '-';
    } else {
        result += ((enPassant & 7) + 97);
        result += (8 - (enPassant >> 3)) + 48;
    }

    result += ' ';
    //half moves
    result += std::to_string(halfMoves);

    result += " 65536";

    return result;
}

void Moment::empty(){
    sides[Black] = 0ULL;
    sides[White] = 0ULL;
    pieces.fill(0ULL);

    toMove = White;
    enPassant = XX;
    castleRights = 15;
    halfMoves = 0;
}

void Moment::setStartPos(){
    sides[Black] = 0xFFFFULL;
    sides[White] = 0xFFFF000000000000ULL;

    pieces[King] = 0x1000000000000010ULL;
    pieces[Queen] = 0x800000000000008ULL;
    pieces[Rook] = 0x8100000000000081ULL;
    pieces[Bishop] = 0x2400000000000024ULL;
    pieces[Knight] = 0x4200000000000042ULL;
    pieces[Pawn] = 0xFF00000000FF00ULL;

    toMove = White;
    enPassant = XX; 
    castleRights = 15;
    halfMoves = 0;    

    beginZobrist();
}

void Moment::print() const{
    std::cout << "sides[0] (black)\tsides[1] (white)\n";
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            std::cout << ((sides[0] >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << "\t\t";
        for (int j = 0; j < 8; j++){
            std::cout << ((sides[1] >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << '\n';
    }

    std::cout << "\n\npieces[0]\tpieces[1]\tpieces[2]\tpieces[3]\tpieces[4]\tpieces[5]\n";
    for (int i = 0; i < 8; i++){
        for (int k = 0; k < 6; k++){
            for (int j = 0; j < 8; j++){
                std::cout << ((pieces[k] >> ((i << 3) + j)) & 1ULL);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    std::cout << "toMove: " << toMove << "\n\n";

    std::cout << "Half Move: " << static_cast<int>(halfMoves) << '\n';

    std::cout << "Castling: " << static_cast<int>(castleRights) << '\n';
    std::cout << "En Passant: " << static_cast<int>(enPassant) << '\n';
}

void Moment::beginZobrist(){
    Bitboard pcs;
    Square f;

    hash = !static_cast<bool>(toMove) * Zobrist::turnKey;

    for (Piece p = King; p < None; p++){
        pcs = sides[Black] & pieces[p]; //Black Pieces
        while (pcs){
            f = popLeastBit(pcs);
            hash ^= Zobrist::pieceKeys[Black][p][f];
        }
    
        pcs = sides[White] & pieces[p];
        while (pcs){
            f = popLeastBit(pcs);
            hash ^= Zobrist::pieceKeys[White][p][f];
        }
    }

    if (enPassant != XX){
        hash ^= Zobrist::passantKeys[enPassant & 7]; //get the file of the ep square
    }

    hash ^= Zobrist::castleKeys[castleRights];
}





