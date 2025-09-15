// Function Definitions for Position

#include "Position.h"

Position::Position(){
    empty();

    plays[0] = Move::Invalid; //must always stay this
}

Position::Position(const Position& p){
    std::cout << "debug -- chungus\n";
    exit(1);
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

    if (isFRC){ 
        restoreCastling(); 
        makeCastleTable({'H', 'A', 'h', 'a'});
    }
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
    halfMoves[0] = 0;

    if (isFRC){
        restoreCastling();
        makeCastleTable({'H', 'A', 'h', 'a'});
    }

    castleRights[0] = 15;

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

    return std::popcount<Bitboard>(pieces[Bishop] | pieces[Knight]) < 2;
}

/*
Bitboard Position::isAttacked(const Square& sq, const Color& c) const{
    Bitboard checkers;
    
    checkers = Attacks::PawnAttacks[!c][sq] & those(c, Pawn);
    if (checkers){ return true; }

    checkers = Attacks::KnightAttacks[sq] & those(c, Knight);
    if (checkers){ return true; }

    checkers = Attacks::KingAttacks[sq] & those(c, King);
    if (checkers){ return true; }

    Bitboard occ = occupied();
    Bitboard army = sides[c];
    checkers = Attacks::rookAttacks(sq, occ) & straightPieces() & army;
    if (checkers){ return true; }

    checkers = Attacks::bishopAttacks(sq, occ) & diagonalPieces() & army;
    return !!checkers;
}
*/

Bitboard Position::isAttacked(const Square& sq, const Color& c) const{
    Bitboard checkers;
    
    checkers = Attacks::PawnAttacks[!c][sq] & those(c, Pawn);
    checkers |= Attacks::KnightAttacks[sq] & those(c, Knight);
    checkers |= Attacks::KingAttacks[sq] & those(c, King);

    Bitboard occ = occupied();
    Bitboard army = sides[c];
    checkers |= Attacks::rookAttacks(sq, occ) & straightPieces() & army;
    checkers |= Attacks::bishopAttacks(sq, occ) & diagonalPieces() & army;

    return checkers;
}

Bitboard Position::isChecked(const Color& c) const{
    Square k = getLeastBit(those(c, King));

    return isAttacked(k, flip(c));
}

/*
bool Position::illegal() const{
    Color us = flip(toMove); //easier to think about from nstm

    uint8_t cat = lastPlayed().castling();
    if (cat){ //Kingside = 1, Queenside = 2, choose the relevant one
        Bitboard extra = (cat - 1) ? queenSafeMask[us] : kingSafeMask[us];
        Square s;
        while (extra){
            s = popLeastBit(extra);
            if (isAttacked(s, flip(us))){
                return true;
            }
        }

        if (isFRC){ //castling rook "blocks"
            Bitboard backRankers = (0xFFULL << (us * 56)) & straightPieces() & sides[!us];
            if (backRankers){
                Square target = lastPlayed().from();
                Square fakeDest = lastPlayed().to(); //king destination
                Square fakeFile = (cat - 1) ? queenRookTo[us] : kingRookTo[us]; //rook destination

                Bitboard occ = (occupied() ^ squareBitboard(fakeDest) ^ squareBitboard(fakeFile));

                Square agro = (cat - 1) ? getLeastBit(backRankers) : getMostBit(backRankers);
                Bitboard seen = Attacks::rookAttacks(agro, occ);
                if (seen & squareBitboard(target)){
                    return true;
                }
            }
        }

        return false;
    }

    return isChecked(us);
}
*/


/*
bool Position::isLegal(const Move& m) const{
    
}
*/

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

    if (isFRC){
        deduceCastling(feed);
    } else { //standard chess can be looked up
        for (ind = 0; ind < 16; ind++){
            if (castleStrings[ind] == feed){
                castleRights[0] = ind;
                break;
            }
        }
    }

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

    result += " 4095";

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

Count Position::repetitions(Index ply) const{
    Count reps = 1; //start with this repetition
    Count ind = clock; // start right now

    while ((ind > 1) and halfMoves[ind]){ //while at the beginning and last 50mr reset
        ind -= 2;
        reps += ((hashes[clock] == hashes[ind]) * (1 + (ind >= clock - ply)));
        //add a rep if the position matches, add another if it occurs after search start
    }

    return reps;
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

    sides[toMove] ^= squareBitboard(start);
    sides[toMove] ^= squareBitboard(end);
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

        sides[toMove] ^= squareBitboard(start);
        sides[toMove] ^= squareBitboard(end);
        pieces[Rook] ^= squareBitboard(start);
        pieces[Rook] ^= squareBitboard(end);

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

    sides[toMove] ^= squareBitboard(start);
    sides[toMove] ^= squareBitboard(end);
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

        sides[toMove] ^= squareBitboard(start);
        sides[toMove] ^= squareBitboard(end);
        pieces[Rook] ^= squareBitboard(start); 
        pieces[Rook] ^= squareBitboard(end);

    }

    clock--;
}

void Position::passMove(){
    clock++;
    plays[clock] = Move::Null;
    castleRights[clock] = castleRights[clock - 1];
    hashes[clock] = hashes[clock - 1] ^ Zobrist::turnKey;

    enPassant[clock] = XX;

    Square lastPassant = enPassant[clock - 1];
    if (lastPassant != XX){
        hashes[clock] ^= Zobrist::passantKeys[lastPassant & 7];
    }

    halfMoves[clock] = halfMoves[clock - 1] + 1;

    toMove = flip(toMove);
}

void Position::unpassMove(){
    toMove = flip(toMove);
    clock--;
}

void Position::deduceCastling(std::string given){
    if (given == "-"){ //nothing can be deduced from no castling rights
        castleRights[clock] = 0;
        return;
    }

    Square wk = static_cast<Square>(getLeastBit(those(White, King)) & 7); //white king file
    Square bk = getLeastBit(those(Black, King)); //black king file
 
    Square rookFile; //file the rook is on as determined by char
    Bitboard rookTravel; //rook travelled squares

    kingRookFrom[Black] = XX; //reset all safety, occupancy, and starting squares
    kingRookFrom[White] = XX;

    queenRookFrom[Black] = XX;
    queenRookFrom[White] = XX;

    kingSafeMask[Black] = 0ULL;
    kingSafeMask[White] = 0ULL;

    queenSafeMask[Black] = 0ULL;
    queenSafeMask[White] = 0ULL;

    kingOccMask[Black] = 0ULL;
    kingOccMask[White] = 0ULL;

    queenOccMask[Black] = 0ULL;
    queenOccMask[White] = 0ULL;

    rightsChange.fill(0);

    castleRights[clock] = 0; //calculate castling rights here as well
    //std::cout << "deduce clock: " << (int)clock << '\n';

    std::array<char, 4> parts = {'Y', 'Z', 'y', 'z'}; //Y = placeholder K, Z = placeholder Q

    //need to update 6 things:
    //clear occupancy, safe mask, rook start, rights change, castling rights, fen strings

    for (char c : given){ //A = 65, H = 72; a = 97, h = 104
        rookFile = static_cast<Square>((c - 1) & 7);

        if (c < 73){ //White has this castling right
            if (wk < rookFile){ //kingside castle
                kingRookFrom[White] = flip(rookFile); //white's kingside rook start (1)

                kingSafeMask[White] = (squareBitboard(h8) - squareBitboard(wk));
                //squares king travels through, inclusive, is also safemask

                rookTravel = (squareBitboard(std::max(rookFile, f8)) << 1)
                    - squareBitboard(std::min(rookFile, f8)); 
                //perhaps replace with a ray table: ray[rookFile][f8] for example

                kingOccMask[White] = kingSafeMask[White] | rookTravel;
                kingOccMask[White] ^= squareBitboard(rookFile);
                kingOccMask[White] ^= squareBitboard(wk);
                //occupancy mask excludes castling king and rook

                kingSafeMask[White] <<= 56; // (2)
                kingOccMask[White] <<= 56; // (3)

                rightsChange[flip(rookFile)] = 1; // rights change (4)
                rightsChange[flip(wk)] |= 1;

                castleRights[clock] |= 1; //white can kingside castle (5)
                //std::cout << "added 1 to castleRights[clock]\n";

                parts[0] = c; // string part (6)        

            } else { //queenside castle
                queenRookFrom[White] = flip(rookFile); // (1)

                queenSafeMask[White] = (wk == b8) ? 0x6ULL : //handle edge case
                    ((squareBitboard(wk) << 1) - squareBitboard(c8));
                
                rookTravel = (squareBitboard(std::max(rookFile, d8)) << 1)
                    - squareBitboard(std::min(rookFile, d8));

                queenOccMask[White] = queenSafeMask[White] | rookTravel;
                queenOccMask[White] ^= squareBitboard(rookFile);
                queenOccMask[White] ^= squareBitboard(wk);

                queenSafeMask[White] <<= 56; // (2)
                queenOccMask[White] <<= 56; // (3)

                rightsChange[flip(rookFile)] = 2; // (4)
                rightsChange[flip(wk)] |= 2;

                castleRights[clock] |= 2;
                
                parts[1] = c;
            }
        } else { //black's castling
            if (bk < rookFile){
                kingRookFrom[Black] = rookFile; // (1)

                kingSafeMask[Black] = squareBitboard(h8) - squareBitboard(bk); // (2)

                rookTravel = (squareBitboard(std::max(rookFile, f8)) << 1)
                    - squareBitboard(std::min(rookFile, f8)); 

                kingOccMask[Black] = kingSafeMask[Black] | rookTravel;
                kingOccMask[Black] ^= squareBitboard(rookFile);
                kingOccMask[Black] ^= squareBitboard(bk); // (3)

                rightsChange[rookFile] = 4; // rights change (4)
                rightsChange[bk] |= 4;

                castleRights[clock] |= 4; //white can kingside castle (5)

                parts[2] = c; // string part (6)  

            } else {
                queenRookFrom[Black] = rookFile; // (1)
                queenSafeMask[Black] = (bk == b8) ? 0x6ULL :
                    ((squareBitboard(bk) << 1) - squareBitboard(c8)); // (2)

                rookTravel = (squareBitboard(std::max(rookFile, d8)) << 1)
                    - squareBitboard(std::min(rookFile, d8));

                queenOccMask[Black] = queenSafeMask[Black] | rookTravel;
                queenOccMask[Black] ^= squareBitboard(rookFile);
                queenOccMask[Black] ^= squareBitboard(bk); // (3)

                rightsChange[rookFile] = 8; // (4)
                rightsChange[bk] |= 8;

                castleRights[clock] |= 8; // (5)

                parts[3] = c; // (6)
            }
        }
    }


    // (6) Creating the Table
    makeCastleTable(parts);
}

void Position::restoreCastling(){
    kingRookFrom[Black] = h8;
    kingRookFrom[White] = h1;
    queenRookFrom[Black] = a8;
    queenRookFrom[White] = a1;
    kingSafeMask[Black] = 0x70ULL;
    kingSafeMask[White] = 0x70ULL << 56;
    queenSafeMask[Black] = 0x1CULL;
    queenSafeMask[White] = 0x1CULL << 56;
    kingOccMask[Black] = 0x60ULL;
    kingOccMask[White] = 0x60ULL << 56;
    queenOccMask[Black] = 0xEULL;
    queenOccMask[White] = 0xEULL << 56;
    rightsChange.fill(0);
    rightsChange[a8] = 8;
    rightsChange[e8] = 12;
    rightsChange[h8] = 4;
    rightsChange[a1] = 2;
    rightsChange[e1] = 3;
    rightsChange[h1] = 1;

    makeCastleTable({'K', 'Q', 'k', 'q'});
    //castleRights[clock] = 0;
}

void Position::makeCastleTable(const std::array<char, 4>& parts){
    for (Index i = 1; i < 16; i++){
        std::string res = "";
        for (int j = 0; j < 4; j++){
            if (i & (1 << j)){ res += parts[j]; }
        }
        castleStrings[i] = res;
    }
}

std::string Position::moveName(const Move& m, bool toFlip) const{
    Square start = m.from();
    Square end;

    Color stm = static_cast<Color>(toMove ^ toFlip);

    if (!m.castling() or !isFRC){
        end = m.to();
    } else if (m.kingCastle()){
        end = kingRookFrom[stm];
    } else {
        end = queenRookFrom[stm];
    }

    std::string result = "";
    result += ((start & 7) + 97);
    result += (8 - (start >> 3)) + 48;

    result += ((end & 7) + 97);
    result += (8 - (end >> 3)) + 48;

    if (m.promoted()){
        result += fenChars[m.ending() + 9];
    }

    return result;
}

