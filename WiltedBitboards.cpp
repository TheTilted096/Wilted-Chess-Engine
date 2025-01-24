/*
Function Definitions for Bitboards Class

TheTilted096, 12-19-24
Wilted Engine
*/

#include "WiltedBitboards.h"

Bitboard Bitboards::RookBoards[0x19000];
Bitboard Bitboards::BishopBoards[0x1480];

Bitboards::Bitboards(){
    empty();
}

Bitboard Bitboards::hqRookAttack(int sq, Bitboard occ){
    Bitboard forward = occ & (0x0101010101010101ULL << (sq & 7));
    Bitboard reverse = __builtin_bswap64(forward);

    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));

    forward ^= __builtin_bswap64(reverse);
    forward &= (0x0101010101010101ULL << (sq & 7)); //forward is now vertical attacks

    //printAsBitboard(forward, std::cout);

    Bitboard horizontal = ((uint64_t)hlt[sq & 7]
        [((((0xFFULL << ((sq & 56))) & occ) >> (sq & 56)) >> 1) & 63]) << (sq & 56);

    //printAsBitboard(horizontal, std::cout);
    //printAsBitboard(forward | horizontal, std::cout);

    return (forward | horizontal);
}

Bitboard Bitboards::rookAttack(int sq, Bitboard occ){
    return RookBoards[RookOffset[sq] + _pext_u64(occ, RookMasks[sq])];
}

Bitboard Bitboards::hqBishopAttack(int sq, Bitboard occ){
    //Position::printAsBitboard(occ);
    Bitboard ldiag = 0x8040201008040201ULL;
    Bitboard rdiag = 0x102040810204080ULL;

    Bitboard sqfile = 0x101010101010101ULL << (sq & 7);
    
    Bitboard forward, reverse;
    //right diagonal
    int d = __builtin_ctzll(sqfile & rdiag) - sq;
    //std::cout << d << '\n';
    Bitboard mask = (d >= 0) ? (rdiag >> d) : (rdiag << -d);
    forward = occ & mask;
    reverse = __builtin_bswap64(forward);
    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));
    forward ^= __builtin_bswap64(reverse);
    forward &= mask;

    Bitboard result = forward; //save right diagonal attacks

    d = __builtin_ctzll(sqfile & ldiag) - sq;
    //std::cout << d << '\n';
    mask = (d >= 0) ? (ldiag >> d) : (ldiag << -d);
    forward = occ & mask;
    reverse = __builtin_bswap64(forward);
    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));
    forward ^= __builtin_bswap64(reverse);
    forward &= mask;

    result |= forward;

    //Position::printAsBitboard(result);

    return result;
}

Bitboard Bitboards::bishopAttack(int sq, Bitboard occ){
    return BishopBoards[BishopOffset[sq] + _pext_u64(occ, BishopMasks[sq])];
}

void Bitboards::initSlideAttacks(){
    int pos = 0;
    Bitboard cons;

    const uint8_t RookBits[64] = 
        {12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12};

    //std::ofstream rt("rawTables.txt");

    for (int sq = 0; sq < 64; sq++){
        for (int idx = 0; idx < (1 << RookBits[sq]); idx++){
            cons = _pdep_u64(idx, RookMasks[sq]) | (1ULL << sq);
            RookBoards[pos] = hqRookAttack(sq, cons);
            //printAsBitboard(RookBoards[pos], rt);
            pos++;
        }
    }

    const uint8_t BishopBits[64] = 
        {6, 5, 5, 5, 5, 5, 5, 6, 
        5, 5, 5, 5, 5, 5, 5, 5, 
        5, 5, 7, 7, 7, 7, 5, 5, 
        5, 5, 7, 9, 9, 7, 5, 5, 
        5, 5, 7, 9, 9, 7, 5, 5, 
        5, 5, 7, 7, 7, 7, 5, 5, 
        5, 5, 5, 5, 5, 5, 5, 5, 
        6, 5, 5, 5, 5, 5, 5, 6};

    pos = 0;

    for (int sq = 0; sq < 64; sq++){
        for (int idx = 0; idx < (1 << BishopBits[sq]); idx++){
            cons = _pdep_u64(idx, BishopMasks[sq]) | (1ULL << sq);
            BishopBoards[pos] = hqBishopAttack(sq, cons);

            pos++;
        }
    }
}

void Bitboards::printAsBitboard(Bitboard board){
    for (int i = 0; i < 64; i++) {
        std::cout << (board & 1ULL);
        if ((i & 7) == 7) {
            std::cout << '\n';
        }
        board >>= 1;
    }
    std::cout << '\n';
}

int Bitboards::pieceAt(int sq){
    Bitboard sqbb = (1ULL << sq);
    for (int i = 0; i < 6; i++){
        if (pieces[i] & sqbb){
            return i;
        }
    }
    return 13; //in case of fail 
    //(but also en passant)
}

void Bitboards::setStartPos(){
    sides[0] = 0xFFFFULL;
    sides[1] = 0xFFFF000000000000ULL;

    pieces[0] = 0x1000000000000010ULL;
    pieces[1] = 0x800000000000008ULL;
    pieces[2] = 0x8100000000000081ULL;
    pieces[3] = 0x2400000000000024ULL;
    pieces[4] = 0x4200000000000042ULL;
    pieces[5] = 0xFF00000000FF00ULL;

    toMove = true;
    thm = 0;
    ep[0] = 255; //out of range en passant because there is none
    cr[0] = 15;    
}

void Bitboards::empty(){
    sides[0] = 0ULL; sides[1] = 0ULL;
    pieces[0] = 0ULL; pieces[1] = 0ULL; pieces[2] = 0ULL;
    pieces[3] = 0ULL; pieces[4] = 0ULL; pieces[5] = 0ULL;

    toMove = true;
    thm = 0;
    ep[0] = 255; //out of range en passant because there is none
    cr[0] = 0;    
}

void Bitboards::print(){
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

    std::cout << "Half Move: " << chm[thm] << '\n';

    std::cout << "Castling: " << (int) cr[thm] << '\n';
    std::cout << "En Passant: " << (int) ep[thm] << "\n\n";
}

void Bitboards::readFen(std::string fen){
    //EXAMPLE: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 moves aa
    empty();

    std::stringstream segs(fen);

    std::string feed;

    segs >> feed; //main information segment
    
    uint8_t ind, sq = 0;
    for (char cc : feed){
        for (ind = 0; ind < 21; ind++){ //use std::find?
            if (frchr[ind] == cc){
                break;
            }
        }

        if ((0 < ind) and (ind < 9)){
            sq += ind;
            continue;
        }
        if ((ind >= 9) and (ind < 15)){
            pieces[ind - 9] |= (1ULL << sq);
            sides[0] |= (1ULL << sq);
            sq++;
            continue;
        }
        if ((ind >= 15) and (ind < 21)){
            pieces[ind - 15] |= (1ULL << sq);
            sides[1] |= (1ULL << sq);
            sq++;
            continue;
        }
    }

    segs >> feed; //side to move
    toMove = feed[0] & 1; //'w' = 119 and 'b' = 98;
    
    segs >> feed; //castling rights 'KQkq' or something
    feed += ' ';
    ind = 0;

    //input total 1/2 move clock starting at 0th
    thm = 0;

    //might want to optimize this later
    cr[0] |= (cstrt[0] == feed[ind]);
    ind += (cstrt[0] == feed[ind]);

    cr[0] |= ((cstrt[1] == feed[ind]) << 1);
    ind += (cstrt[1] == feed[ind]);

    cr[0] |= ((cstrt[2] == feed[ind]) << 2);
    ind += (cstrt[2] == feed[ind]);

    cr[0] |= ((cstrt[3] == feed[ind]) << 3);
    ind += (cstrt[3] == feed[ind]);

    segs >> feed; //en passant square
    ep[0] = (feed != "-") ? ((feed[0] - 97) + 8 * (56 - feed[1])) : 255;

    //implement half move clock
    segs >> feed;
    chm[0] = std::stoi(feed); 
}

