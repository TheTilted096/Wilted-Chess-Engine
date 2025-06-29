// Function Definitions for Position

#include "Position.h"

Position::Position(){
    empty();

    plays[0] = Move::Invalid; //must always stay this
}

Position::Position(const Position& p){
    sides = p.sides;
    pieces = p.pieces;

    toMove = p.toMove;

    castleRights = p.castleRights;
    enPassant = p.enPassant;
    halfMoves = p.halfMoves;
    hashes = p.hashes;
    plays = p.plays;

    clock = p.clock;
}

void Position::empty(){
    sides[Black] = 0ULL;
    sides[White] = 0ULL;
    pieces.fill(0ULL);

    clock = 0;

    toMove = White;
    enPassant[0] = XX;
    castleRights[0] = 0;
    halfMoves[0] = 0;
}

void Position::setStartPos(){
    sides[Black] = 0xFFFFULL;
    sides[White] = 0xFFFF000000000000ULL;

    pieces[King] = 0x1000000000000010ULL;
    pieces[Queen] = 0x800000000000008ULL;
    pieces[Rook] = 0x8100000000000081ULL;
    pieces[Bishop] = 0x2400000000000024ULL;
    pieces[Knight] = 0x4200000000000042ULL;
    pieces[Pawn] = 0xFF00000000FF00ULL;

    toMove = White;

    clock = 0;

    enPassant[0] = XX; 
    castleRights[0] = 15;
    halfMoves[0] = 0;    

    beginZobrist();
}

Piece Position::pieceAt(const Square& s) const{
    Bitboard sqbb = squareBitboard(s);

    Piece p = King;
    for (const Bitboard& b : pieces){
        if (sqbb & b){ return p; }
        p++;
    }
    return p;
}

bool Position::insufficient() const{
    if (pieces[Queen] | pieces[Rook] | pieces[Pawn]){
        return false;
    }

    return std::popcount<Bitboard>((pieces[Bishop] | pieces[Knight]) < 2);
}

void Position::readFen(std::string fen){
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
        if (castleStrings[ind] == feed){
            break;
        }
    }
    castleRights[0] = ind;

    segments >> feed; //EP Square
    enPassant[0] = (feed != "-") ? static_cast<Square>((feed[0] - 97) + 8 * (56 - feed[1])) : XX;

    //Half Move Clock
    segments >> feed;
    halfMoves[0] = std::stoi(feed);    

    beginZobrist();
}

std::string Position::makeFen() const{
    Bitboard occ = sides[White] | sides[Black];
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
    result += castleStrings[castleRights[clock]];
    result += ' ';

    if (enPassant[clock] == XX){
        result += '-';
    } else {
        result += ((enPassant[clock] & 7) + 97);
        result += (8 - (enPassant[clock] >> 3)) + 48;
    }

    result += ' ';
    //half moves
    result += std::to_string(halfMoves[clock]);

    result += " 65536";

    return result;
}

void Position::print() const{
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

    std::cout << "Half Move: " << static_cast<int>(halfMoves[clock]) << '\n';

    std::cout << "Castling: " << static_cast<int>(castleRights[clock]) << '\n';
    std::cout << "En Passant: " << static_cast<int>(enPassant[clock]) << '\n';
    std::cout << "Clock: " << static_cast<int>(clock) << '\n';
    std::cout << makeFen() << "\n\n";
}

void Position::beginZobrist(){
    Bitboard pcs;
    Square f;

    hashes[clock] = !static_cast<bool>(toMove) * Zobrist::turnKey;

    for (Piece p = King; p < None; p++){
        pcs = sides[Black] & pieces[p]; //Black Pieces
        while (pcs){
            f = popLeastBit(pcs);
            hashes[clock] ^= Zobrist::pieceKeys[Black][p][f];
        }
    
        pcs = sides[White] & pieces[p];
        while (pcs){
            f = popLeastBit(pcs);
            hashes[clock] ^= Zobrist::pieceKeys[White][p][f];
        }
    }

    if (enPassant[clock] != XX){
        hashes[clock] ^= Zobrist::passantKeys[enPassant[clock] & 7]; //get the file of the ep square
    }

    hashes[clock] ^= Zobrist::castleKeys[castleRights[clock]];
}

void Position::showZobrist() const{
    std::cout << "Zobrist History:\n";
    for (Count i = 0; i <= clock; i++){
        std::cout << "ZH " << static_cast<int>(i) << ": " << hashes[i] << "\tHalves: " << static_cast<int>(halfMoves[i]) << '\n';
    }
}

void Position::forget(){
    castleRights[0] = castleRights[clock];
    enPassant[0] = enPassant[clock];
    halfMoves[0] = halfMoves[clock];
    hashes[0] = hashes[clock];

    clock = 0;
}

void Position::makeMove(const Move& m){
    Square start = m.from();
    Square end = m.to();

    Piece typei = m.moving();
    Piece typef = m.ending();

    Piece victim = m.captured();

    clock++;

    plays[clock] = m;

    castleRights[clock] = castleRights[clock - 1];
    hashes[clock] = hashes[clock - 1] ^ Zobrist::turnKey;

    if (victim){
        bool passant = m.epCapture();

        Square target = static_cast<Square>(end + passant * ((toMove << 4) - 8));

        pieces[victim] ^= squareBitboard(target);
        sides[!toMove] ^= squareBitboard(target);

        hashes[clock] ^= Zobrist::pieceKeys[!toMove][victim][target];
    }

    sides[toMove] ^= (squareBitboard(start) | squareBitboard(end));
    pieces[typei] ^= squareBitboard(start);
    pieces[typef] ^= squareBitboard(end);

    hashes[clock] ^= Zobrist::pieceKeys[toMove][typei][start];
    hashes[clock] ^= Zobrist::pieceKeys[toMove][typef][end];

    if (m.doublePushed()){
        enPassant[clock] = static_cast<Square>(end - 8 + (toMove << 4));
        hashes[clock] ^= Zobrist::passantKeys[end & 7];
    } else {
        enPassant[clock] = XX;
    }

    Square lastPassant = enPassant[clock - 1];
    if (lastPassant != XX){
        hashes[clock] ^= Zobrist::passantKeys[lastPassant & 7];
    }

    if (victim or (typei == Pawn)){
        halfMoves[clock] = 0;
    } else {
        halfMoves[clock] = halfMoves[clock - 1] + 1;
    }

    if (castleRights[clock]){
        uint8_t change = rightsChange[start] | rightsChange[end];
        if (change){
            castleRights[clock] &= ~change;
            hashes[clock] ^= Zobrist::castleKeys[castleRights[clock - 1]]; //replace zobrist key
            hashes[clock] ^= Zobrist::castleKeys[castleRights[clock]];
        }
    }

    if (m.castling()){
        if (m.kingCastle()){ //kingside castle 
            start = kingRookFrom[toMove];
            end = kingRookTo[toMove];
        } else {
            start = queenRookFrom[toMove];
            end = queenRookTo[toMove];
        }

        sides[toMove] ^= (squareBitboard(start) | squareBitboard(end));
        pieces[Rook] ^= (squareBitboard(start) | squareBitboard(end));

        hashes[clock] ^= Zobrist::pieceKeys[toMove][Rook][start];
        hashes[clock] ^= Zobrist::pieceKeys[toMove][Rook][end];
    }

    toMove = flip(toMove);    
}

void Position::unmakeMove(){
    Move& m = plays[clock];

    Square start = m.from();
    Square end = m.to();

    Piece typei = m.moving();
    Piece typef = m.ending();

    Piece victim = m.captured();

    toMove = flip(toMove); //original stm

    sides[toMove] ^= (squareBitboard(start) | squareBitboard(end));
    pieces[typei] ^= squareBitboard(start);
    pieces[typef] ^= squareBitboard(end);

    if (victim){
        bool passant = m.epCapture();
        Square target = static_cast<Square>(end + passant * ((toMove << 4) - 8));

        pieces[victim] ^= squareBitboard(target);
        sides[!toMove] ^= squareBitboard(target);
    }

    if (m.castling()){
        if (m.kingCastle()){ //kingside castle 
            start = kingRookFrom[toMove];
            end = kingRookTo[toMove];
        } else {
            start = queenRookFrom[toMove];
            end = queenRookTo[toMove];
        }

        sides[toMove] ^= (squareBitboard(start) | squareBitboard(end));
        pieces[Rook] ^= (squareBitboard(start) | squareBitboard(end));

    }

    clock--;
}




