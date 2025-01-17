/*
Function Definitions for Position Class

TheTilted096, 12-19-24
Wilted Engine
*/

#include "WiltedPosition.h"

Move::Move(){
    info = 0;
}

Move::Move(uint32_t& m){
    info = m;
}

inline bool Move::prmt(){ return (((info >> 15) & 7U) != ((info >> 18) & 7U)); }

inline int Move::stsq(){ return (info & 63U); }

inline int Move::edsq(){ return (info >> 6) & 63U; }

inline bool Move::capt(){ return (info >> 12) & 7U; }

inline int Move::cptp(){ return (info >> 12) & 7U; }

inline int Move::tpmv(){ return (info >> 15) & 7U; }

inline int Move::tpnd(){ return (info >> 18) & 7U; }

inline int Move::cstl(){ return (info >> 21) & 3U; }

inline bool Move::dpsh(){ return (info >> 23) & 1U; }

inline bool Move::epcp(){ return (info >> 24) & 1U; }

std::string Move::toStr(){
    uint8_t start = info & 63U;
    uint8_t end = (info >> 6) & 63U;

    std::string result;

    result += ((start & 7) + 97);
    result += (8 - (start >> 3)) + 48;

    result += ((end & 7) + 97);
    result += (8 - (end >> 3)) + 48;

    if (prmt()){
        result += Bitboards::frchr[9 + tpnd()];
    }

    return result;
}

void Move::print(){
    std::cout << info << '\n';
}

/*
inline void Move::operator|=(const uint32_t& t){
    info |= t;
}

inline void Move::operator=(const uint32_t& t){
    info = t;
}
*/

Position::Position(){
    //nothing for now
}

bool Position::isAttacked(int sq, bool s){
    Bitboard inc;

    inc = plt[!s][sq] & pieces[5] & sides[s];
    inc |= KnightAttacks[sq] & pieces[4] & sides[s];

    Bitboard occ = sides[0] | sides[1] | (1ULL << sq);
    Bitboard bpst = bishopAttack(sq, occ);

    inc |= bpst & (pieces[3] | pieces[1]) & sides[s];

    Bitboard rkst = rookAttack(sq, occ);

    inc |= rkst & (pieces[2] | pieces[1]) & sides[s];

    inc |= KingAttacks[sq] & pieces[0] & sides[s];

    //cm = inc;
    
    return inc;
}

bool Position::isChecked(bool s){
    return isAttacked(__builtin_ctzll(sides[s] & pieces[0]), !s);
}

bool Position::notValid(Move m){
    uint8_t cat = m.cstl(); //Q = 2, K = 1
    if (cat){
        //std::cout << "Castle: " << (int) cat << '\n';
        Bitboard cmask = (0x1C0ULL >> (cat << 1)) << (!toMove * 56);
        //printAsBitboard(cmask);
        int f;
        while (cmask){
            f = __builtin_ctzll(cmask);
            //std::cout << "Testing: " << f << '\n';

            if (isAttacked(f, toMove)){
                //std::cout << "Failed " << f << '\n';
                return true;
            }

            cmask ^= (1ULL << f);
        }

        return false;
    }

    return isChecked(!toMove);
}

/*
Move Representation:

0000 0 0 0 0 
0 000 0 000 000 0 000000 000000

0-5: start square
6-11: end square

12-14: Captured Type
15-17: Piece Type Moved

18-20: Piece End Type

[New Stuff]
21: Castle Kingside
22: Castle Queenside

23: Double Pawn Push
24: En Passant Capture
*/

int Position::generateMoves(int ply){
    int tgm = 0;
    Bitboard mvst, pcs, xset;
    Bitboard occ = sides[0] | sides[1];
    Bitboard ours = sides[toMove];
    Bitboard opps = sides[!toMove];

    int f, p, cc;

    //Pawns

    pcs = pieces[5] & ours;

    Bitboard pshtrgt = ((pcs << 8) >> (toMove << 4)) & ~occ;
    Bitboard dpshtrgt = ((pshtrgt << 8) >> (toMove << 4)) & ~occ & (0xFF000000ULL << (toMove << 3)); //two-square pushes

    //Pawn Captures
    while (pcs){
        f = __builtin_ctzll(pcs);

        mvst = plt[toMove][f] & (opps | ((ep[thm] != 255) * (1ULL << ep[thm]))); //add ep square to pawn caps

        while (mvst){
            p = __builtin_ctzll(mvst);

            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);

            cc = pieceAt(p);
            moves[ply][tgm].info |= (cc << 12); //captured type
            moves[ply][tgm].info |= ((cc == 13) << 24); //ep capture

            moves[ply][tgm].info |= (5U << 15); //type moved

            bool prmt = ((1ULL << p) & 0xFF000000000000FF); //cast promotion mask to bool
            if (prmt){
                uint32_t tmp = moves[ply][tgm].info;

                moves[ply][tgm].info = tmp | (1U << 18);
                tgm++;

                moves[ply][tgm].info = tmp | (2U << 18);
                tgm++;

                moves[ply][tgm].info = tmp | (3U << 18);
                tgm++;

                moves[ply][tgm].info = tmp | (4U << 18);
            } else {
                moves[ply][tgm].info |= (5U << 18);
            }

            mvst ^= (1ULL << p);
            tgm++;
        }

        pcs ^= (1ULL << f);       
    }

    //pawn pushes and double-pushes
    while (pshtrgt){
        f = __builtin_ctzll(pshtrgt);
        moves[ply][tgm].info = f - 8 + (toMove << 4); //start
        moves[ply][tgm].info |= (f << 6); //end

        moves[ply][tgm].info |= (5U << 15); //type moved

        bool prmt = ((1ULL << f) & 0xFF000000000000FF); //cast promotion mask to bool
        if (prmt){
            uint32_t tmp = moves[ply][tgm].info;

            moves[ply][tgm].info = tmp | (1U << 18);
            tgm++;

            moves[ply][tgm].info = tmp | (2U << 18);
            tgm++;

            moves[ply][tgm].info = tmp | (3U << 18);
            tgm++;

            moves[ply][tgm].info = tmp | (4U << 18);
        } else {
            moves[ply][tgm].info |= (5U << 18);
        }

        pshtrgt ^= (1ULL << f);
        tgm++;
    }

    while (dpshtrgt){
        f = __builtin_ctzll(dpshtrgt);
        moves[ply][tgm].info = f - 16 + (toMove << 5);
        moves[ply][tgm].info |= (f << 6);
        moves[ply][tgm].info |= (5U << 15);

        moves[ply][tgm].info |= (5U << 18);
        moves[ply][tgm].info |= (1U << 23);

        dpshtrgt ^= (1ULL << f);
        tgm++;
    }

    //Knights
    pcs = pieces[4] & ours;
    while (pcs){
        f = __builtin_ctzll(pcs);

        mvst = KnightAttacks[f] & ~ours;
        xset = mvst & opps;
        mvst ^= xset;

        while (xset){
            p = __builtin_ctzll(xset);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);
            
            cc = pieceAt(p);
            moves[ply][tgm].info |= (cc << 12);

            moves[ply][tgm].info |= (4U << 15);
            moves[ply][tgm].info |= (4U << 18);

            tgm++;
            xset ^= (1ULL << p);
        }

        while (mvst){
            p = __builtin_ctzll(mvst);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);

            moves[ply][tgm].info |= (4U << 15);
            moves[ply][tgm].info |= (4U << 18);
            
            tgm++;
            mvst ^= (1ULL << p);
        }

        pcs ^= (1ULL << f);
    }

    //Bishops
    pcs = pieces[3] & ours;
    while (pcs){
        f = __builtin_ctzll(pcs);

        mvst = bishopAttack(f, occ) & ~ours;
        xset = mvst & opps;
        mvst ^= xset;

        while (xset){
            p = __builtin_ctzll(xset);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);
            
            cc = pieceAt(p);
            moves[ply][tgm].info |= (cc << 12);

            moves[ply][tgm].info |= (3U << 15);
            moves[ply][tgm].info |= (3U << 18);

            tgm++;
            xset ^= (1ULL << p);
        }

        while (mvst){
            p = __builtin_ctzll(mvst);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);

            moves[ply][tgm].info |= (3U << 15);
            moves[ply][tgm].info |= (3U << 18);
            
            tgm++;
            mvst ^= (1ULL << p);
        }

        pcs ^= (1ULL << f);
    }

    //Rooks
    pcs = pieces[2] & ours;
    while (pcs){
        f = __builtin_ctzll(pcs);

        mvst = rookAttack(f, occ) & ~ours;
        xset = mvst & opps;
        mvst ^= xset;

        while (xset){
            p = __builtin_ctzll(xset);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);
            
            cc = pieceAt(p);
            moves[ply][tgm].info |= (cc << 12);

            moves[ply][tgm].info |= (2U << 15);
            moves[ply][tgm].info |= (2U << 18);

            tgm++;
            xset ^= (1ULL << p);
        }

        while (mvst){
            p = __builtin_ctzll(mvst);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);

            moves[ply][tgm].info |= (2U << 15);
            moves[ply][tgm].info |= (2U << 18);
            
            tgm++;
            mvst ^= (1ULL << p);
        }

        pcs ^= (1ULL << f);
    }

    //Queens
    pcs = pieces[1] & ours;
    while (pcs){
        f = __builtin_ctzll(pcs);

        mvst = (bishopAttack(f, occ) | rookAttack(f, occ)) & ~ours;
        xset = mvst & opps;
        mvst ^= xset;

        while (xset){
            p = __builtin_ctzll(xset);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);
            
            cc = pieceAt(p);
            moves[ply][tgm].info |= (cc << 12);

            moves[ply][tgm].info |= (1U << 15);
            moves[ply][tgm].info |= (1U << 18);

            tgm++;
            xset ^= (1ULL << p);
        }

        while (mvst){
            p = __builtin_ctzll(mvst);
            moves[ply][tgm].info = f;
            moves[ply][tgm].info |= (p << 6);

            moves[ply][tgm].info |= (1U << 15);
            moves[ply][tgm].info |= (1U << 18);
            
            tgm++;
            mvst ^= (1ULL << p);
        }

        pcs ^= (1ULL << f);
    }

    //King
    pcs = pieces[0] & ours;
    f = __builtin_ctzll(pcs);

    mvst = KingAttacks[f] & ~ours;
    xset = mvst & opps;
    mvst ^= xset;

    while (xset){
        p = __builtin_ctzll(xset);
        moves[ply][tgm].info = f;
        moves[ply][tgm].info |= (p << 6);
        
        cc = pieceAt(p);
        moves[ply][tgm].info |= (cc << 12);

        //moves[ply][tgm].info |= (0U << 15);
        //moves[ply][tgm].info |= (0U << 18);

        tgm++;
        xset ^= (1ULL << p);
    }

    while (mvst){
        p = __builtin_ctzll(mvst);
        moves[ply][tgm].info = f;
        moves[ply][tgm].info |= (p << 6);

        //moves[ply][tgm].info |= (0U << 15);
        //moves[ply][tgm].info |= (0U << 18);
        
        tgm++;
        mvst ^= (1ULL << p);
    }

    //King Castling (Standard for Now)

    Bitboard cmask = 0x60ULL << (56 * toMove);
    bool able = !(cmask & occ) and (cr[thm] & (1 << (!toMove << 1)));
    if (able){
        moves[ply][tgm].info = f;
        moves[ply][tgm].info |= ((6 + 56 * toMove) << 6);
        moves[ply][tgm].info |= (1U << 21);
        tgm++;
    }

    cmask = 0xEULL << (56 * toMove);
    able = !(cmask & occ) and (cr[thm] & (2 << (!toMove << 1)));
    if (able){
        moves[ply][tgm].info = f;
        moves[ply][tgm].info |= ((2 + 56 * toMove) << 6);
        moves[ply][tgm].info |= (1U << 22);
        tgm++;
    }

    return tgm;
}

void Position::makeRMove(Move m){
    uint8_t startsquare = m.stsq();
    uint8_t endsquare = m.edsq();

    uint8_t typeMoved = m.tpmv();
    uint8_t typeEnded = m.tpnd();

    bool capturing = m.capt();
    //uint8_t captureType = m.cptp();

    bool passed = m.epcp();

    //Branchless Capture Code
    /*
    uint8_t target = (endsquare + passed * ((toMove << 4) - 8));
    pieces[captureType] ^= capturing * (1ULL << target);
    sides[!toMove] ^= capturing * (1ULL << target);
    */

    if (capturing){
        uint8_t target = (endsquare + passed * ((toMove << 4) - 8));
        pieces[m.cptp()] ^= (1ULL << target);
        sides[!toMove] ^= (1ULL << target);
    }
    

    sides[toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));
    pieces[typeMoved] ^= (1ULL << startsquare);
    pieces[typeEnded] ^= (1ULL << endsquare);

    thm++;

    //En Passant Square Update
    ep[thm] = m.dpsh() * (endsquare - 7 + (toMove << 4)) - 1;

    //Adjust Castling Rights
    cr[thm] = cr[thm - 1]; //castling rights preserved at first

    if (typeMoved == 0){ //King Move Nullifies
        cr[thm] &= (0xF3 >> (toMove << 1));
    }

    /*
    uint8_t csq = (cf[0] & 0xF) + 56 * toMove; //kingside
    if ((startsquare == csq) or (endsquare == csq)){
        cr[thm] &= (0xFB >> (toMove << 1));
    }

    csq = (cf[1] & 0xF) + 56 * toMove; //queenside
    if ((startsquare == csq) or (endsquare == csq)){
        cr[thm] &= (0xF7 >> (toMove << 1));
    }
    */

    uint8_t csq = (cf[0] & 0xF) + 56 * toMove; //our kingside square
    if (startsquare == csq){
        cr[thm] &= (0xFB >> (toMove << 1));
    }

    if (endsquare == (csq ^ 56)){
        cr[thm] &= (0xFB >> (!toMove << 1));
    }

    csq = (cf[1] & 0xF) + 56 * toMove; //queenside
    if (startsquare == csq){
        cr[thm] &= (0xF7 >> (toMove << 1));
    }

    if (endsquare == (csq ^ 56)){
        cr[thm] &= (0xF7 >> (!toMove << 1));
    }

    //castling
    //still have to play Rh1-f1 or Ra1-d1
    uint8_t cat = m.cstl(); //valued 1 K or 2 Q
    if (cat){
        startsquare = (cf[cat - 1] & 0xF) + 56 * toMove;
        endsquare = (cf[cat - 1] >> 4) + 56 * toMove;

        sides[toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));
        pieces[2] ^= ((1ULL << startsquare) | (1ULL << endsquare)); //move the rook
    }

    toMove ^= 1;
    
}

void Position::unmakeRMove(Move m){
    uint8_t startsquare = m.stsq();
    uint8_t endsquare = m.edsq();

    uint8_t typeMoved = m.tpmv();
    uint8_t typeEnded = m.tpnd();

    bool capturing = m.capt();
    //uint8_t captureType = m.cptp();

    bool passed = m.epcp();

    sides[!toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare)); 
    pieces[typeMoved] ^= (1ULL << startsquare);
    pieces[typeEnded] ^= (1ULL << endsquare);

    /*Branchless Capture Code
    uint8_t target = (endsquare + passed * ((!toMove << 4) - 8));
    pieces[captureType] ^= capturing * (1ULL << target);
    sides[toMove] ^= capturing * (1ULL << target);
    */
    
    if (capturing){
        uint8_t target = (endsquare + passed * ((!toMove << 4) - 8));
        pieces[m.cptp()] ^= (1ULL << target);
        sides[toMove] ^= (1ULL << target);
    }

    thm--;

    uint8_t cat = m.cstl();
    if (cat){
        startsquare = (cf[cat - 1] >> 4) + 56 * !toMove;
        endsquare = (cf[cat - 1] & 0xF) + 56 * !toMove;

        sides[!toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));
        pieces[2] ^= ((1ULL << startsquare) | (1ULL << endsquare)); //move the rook
    }

    toMove ^= 1;
}

uint64_t Position::perft(int depth, int ply){
    uint64_t pnodes = 0;
    if (depth == 0){
        return 1ULL;
    }

    int v = generateMoves(ply);
    for (int i = 0; i < v; i++){
        
        //if (ply == 0){ std::cout << "\n\n";}
        
        //if (ply == 0){
        //    std::cout << "Making: " << ply << ' ' << moves[ply][i].toStr() << ": " 
        //        << moves[ply][i].info << '\n';
        //}
        
        /*
        if (moves[ply][i].toStr() == "e1g1" and (ply == 0)){
            print();
        }
        */
        makeRMove(moves[ply][i]);
        /*
        if (moves[ply][i].toStr() == "e1g1" and (ply == 0)){
            print();
        }
        */
        
        //std::cout << "EP Square: " << (int) ep[thm] << '\n';

        //std::cout << "Checking Legality\n";

        if (notValid(moves[ply][i])){
            //if (ply == 0) { 
                //std::cout << "Legality Prune\n"; 
                //print();
            //    printAsBitboard(cm);
            //}
            unmakeRMove(moves[ply][i]);
            continue;
        }

        uint64_t next = perft(depth -1, ply + 1);

        //if (ply < 2){
        //    std::cout << "UNmaking: " << moves[ply][i].toStr() << ": " 
        //        << moves[ply][i].info << '\n';
        //}
        unmakeRMove(moves[ply][i]);

        //if (moves[ply][i].toStr() == "e1g1"){
        //    print();
        //}
        
        
        if (ply == 0){
            std::cout << ply << ": " << moves[ply][i].toStr() << ": " 
                << moves[ply][i].info << ": " << next << '\n';

            //print();
        }
        

        pnodes += next;
    }

    if (ply == 0){
        std::cout << '\n' << pnodes << " nodes\n";
    }

    return pnodes;
}

















