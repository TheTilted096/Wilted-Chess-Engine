/*
Class Definition of Position
Intermediate Object

Contains static evaluation and advanced makeMove

*/

#include "WiltedBitboards.cpp"

class Move{
    public:
        uint32_t info;

        Move();
        Move(const uint32_t&);
        //Move(uint32_t&);
        Move(const Move&);

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

        bool operator==(const Move&);

        //ton of other things
        
};

/*
Function Definitions for Position Class

TheTilted096, 12-19-24
Wilted Engine
*/

Move::Move(){
    info = 0;
}

Move::Move(const uint32_t& m){
    info = m;
}

Move::Move(const Move& m){ info = m.info; }

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

bool Move::operator==(const Move& m){
    return (m.info == info);
}

/*
inline void Move::operator|=(const uint32_t& t){
    info |= t;
}

inline void Move::operator=(const uint32_t& t){
    info = t;
}
*/

class Position : public Bitboards{
    public:
        Move moves[64][128];
        int mprior[64][128];

        int scores[2];//, eScores[2];
        //int mobil[2], eMobil[2];
        //int gamePhase;

        //total game phase
        //phases, material values

        static constexpr int material[6] = {0, 900, 500, 350, 300, 100};

        //mobilities

        //Bitboard atktbl[2][5];

        uint64_t nodes;
        
        //piece-square tables

        int mps[6][64] = 
        {{6, 21, 0, 30, 24, 10, 20, 8, 
        -24, -31, 11, -3, 39, 24, 10, -32, 
        -34, -13, -23, -39, 5, -5, 22, -6, 
        36, -7, 29, 4, -24, -12, -9, 26, 
        -39, 23, 17, 38, -38, -26, -17, -5, 
        0, 2, -1, -4, 38, 10, -30, -5, 
        -16, 37, 9, -12, 15, -34, 13, 26, 
        -27, -1, 18, 2, -23, -15, -4, -33},

        {-77, -27, 49, -64, 32, 67, -43, -39, 
        -90, -35, 5, 5, -88, -8, -48, 66, 
        42, -45, -16, 23, -22, -42, -89, 89, 
        47, 84, 39, -81, 69, -65, -49, 84, 
        53, -32, -61, 46, -9, -90, 46, -83, 
        77, 27, 50, -47, 81, 29, -4, -19, 
        24, 55, 49, 36, -71, -12, 36, -27, 
        8, -61, 37, 59, 75, -48, 31, 25},

        {-44, -19, 33, -8, -7, 17, 29, -36, 
        -7, -43, 46, -33, 35, 0, -10, -34, 
        -25, 20, -1, 36, 32, 37, -41, 18, 
        40, -6, 33, 24, 39, -26, 38, 44, 
        30, 25, 12, -40, 18, -5, 46, 20, 
        -16, -44, -32, -10, 26, 40, -16, 13, 
        29, 0, -43, -4, -11, -38, -13, 11, 
        32, 6, -41, 2, 15, 6, 13, -19},

        {-8, -33, 20, 29, -20, -29, -24, 25, 
        12, 14, -29, -30, 11, -35, 14, 17, 
        -21, -4, -32, -1, 26, -16, 19, 3, 
        21, -30, 4, -30, 5, -15, -3, 9, 
        27, 30, 15, -19, -1, 13, -15, 19, 
        -3, 10, 25, -1, 1, -2, -26, 2, 
        -2, -4, 0, -16, 5, -32, -28, 2, 
        -31, 19, 27, -18, -1, -16, 2, 12},

        {25, 20, 9, -8, 25, 18, -6, 16, 
        -8, 20, 19, 24, -9, 26, 25, 22, 
        -8, 29, -28, -17, -11, -10, 22, 14, 
        0, 22, 11, 14, 28, -12, 28, 0, 
        18, -22, -11, -11, -11, 23, 4, 0, 
        -13, 29, 7, 15, -19, 0, -8, -7, 
        9, 23, 7, -21, 22, 25, -8, -5, 
        23, 20, 7, -19, -8, -26, 16, 22},

        {0, 0, 0, 0, 0, 0, 0, 0,
        1, -6, -8, -10, 4, -7, -1, -9, 
        -1, 4, 2, 9, 1, 8, -10, 8, 
        -2, 7, 8, 4, 4, -10, 3, -7, 
        -3, -5, -6, 0, -6, 2, 7, -4, 
        -1, 0, -8, -5, -4, -10, 8, 2, 
        -9, -4, -6, 6, 7, 2, 2, 9, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        Position();

        bool isAttacked(int, bool);
        bool isChecked(bool);
        bool notValid(Move);

        int evaluate();
        int evaluateScratch();

        int generateMoves(int);
        int generateCaptures(int);

        uint64_t perft(int, int);

        template <bool> void makeMove(Move);
        template <bool> void unmakeMove(Move);

        void sendMove(std::string);


};

Position::Position(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            mps[i][j] += material[i];
        }
    }
    nodes = 0ULL;
}

bool Position::isAttacked(int sq, bool s){
    /*
    Bitboard inc;

    inc = plt[!s][sq] & pieces[5] & sides[s];
    inc |= KnightAttacks[sq] & pieces[4] & sides[s];

    Bitboard occ = sides[0] | sides[1]; // | (1ULL << sq);
    Bitboard bpst = bishopAttack(sq, occ);

    inc |= bpst & (pieces[3] | pieces[1]) & sides[s];

    Bitboard rkst = rookAttack(sq, occ);

    inc |= rkst & (pieces[2] | pieces[1]) & sides[s];

    inc |= KingAttacks[sq] & pieces[0] & sides[s];

    //cm = inc;
    
    return inc;
    */

    Bitboard inc;

    inc = plt[!s][sq] & pieces[5] & sides[s];
    if (inc){ return true; }

    inc = KnightAttacks[sq] & pieces[4] & sides[s];
    if (inc){ return true; }

    Bitboard occ = sides[0] | sides[1];
    inc = bishopAttack(sq, occ) & (pieces[3] | pieces[1]) & sides[s];
    if (inc){ return true; }
    
    inc = rookAttack(sq, occ) & (pieces[2] | pieces[1]) & sides[s];
    if (inc){ return true; }

    inc = KingAttacks[sq] & pieces[0] & sides[s];
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

   	Bitboard pawnopps = opps | ((ep[thm] != 255) * (1ULL << ep[thm])); //maybe ep[thm] + 1 = 0?

	Bitboard leftcap = (((pcs & 0xFEFEFEFEFEFEFEFEULL) << 7) >> (toMove << 4)) & pawnopps;
	Bitboard rightcap = (((pcs & 0x7F7F7F7F7F7F7F7FULL) << 9) >> (toMove << 4)) & pawnopps;

	while (rightcap){
		f = __builtin_ctzll(rightcap);

		moves[ply][tgm].info = f - 9 + (toMove << 4);
		moves[ply][tgm].info |= (f << 6);
		
		cc = pieceAt(f);
		moves[ply][tgm].info |= (cc << 12);
		moves[ply][tgm].info |= ((cc == 13) << 24); //ep capture;

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

		rightcap ^= (1ULL << f);
		tgm++;
	}

	while (leftcap){
		f = __builtin_ctzll(leftcap);

		moves[ply][tgm].info = f - 7 + (toMove << 4);
		moves[ply][tgm].info |= (f << 6);
		
		cc = pieceAt(f);
		moves[ply][tgm].info |= (cc << 12);
		moves[ply][tgm].info |= ((cc == 13) << 24); //ep capture;

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

		leftcap ^= (1ULL << f);
		tgm++;
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

int Position::generateCaptures(int ply){
    int tgm = 0;
    Bitboard mvst, pcs, xset;
    Bitboard occ = sides[0] | sides[1];
    Bitboard ours = sides[toMove];
    Bitboard opps = sides[!toMove];

    int f, p, cc;

    //Pawns

    pcs = pieces[5] & ours;

    //Bitboard pshtrgt = ((pcs << 8) >> (toMove << 4)) & ~occ;
    //Bitboard dpshtrgt = ((pshtrgt << 8) >> (toMove << 4)) & ~occ & (0xFF000000ULL << (toMove << 3)); //two-square pushes

   	Bitboard pawnopps = opps | ((ep[thm] != 255) * (1ULL << ep[thm])); //maybe ep[thm] + 1 = 0?

	Bitboard leftcap = (((pcs & 0xFEFEFEFEFEFEFEFEULL) << 7) >> (toMove << 4)) & pawnopps;
	Bitboard rightcap = (((pcs & 0x7F7F7F7F7F7F7F7FULL) << 9) >> (toMove << 4)) & pawnopps;

	while (rightcap){
		f = __builtin_ctzll(rightcap);

		moves[ply][tgm].info = f - 9 + (toMove << 4);
		moves[ply][tgm].info |= (f << 6);
		
		cc = pieceAt(f);
		moves[ply][tgm].info |= (cc << 12);
		moves[ply][tgm].info |= ((cc == 13) << 24); //ep capture;

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

		rightcap ^= (1ULL << f);
		tgm++;
	}

	while (leftcap){
		f = __builtin_ctzll(leftcap);

		moves[ply][tgm].info = f - 7 + (toMove << 4);
		moves[ply][tgm].info |= (f << 6);
		
		cc = pieceAt(f);
		moves[ply][tgm].info |= (cc << 12);
		moves[ply][tgm].info |= ((cc == 13) << 24); //ep capture;

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

		leftcap ^= (1ULL << f);
		tgm++;
	}
	
    //pawn pushes and double-pushes
    /*
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
    */

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

    return tgm;
}

template <bool ev> void Position::makeMove(Move m){
    uint8_t startsquare = m.stsq();
    uint8_t endsquare = m.edsq();

    uint8_t typeMoved = m.tpmv();
    uint8_t typeEnded = m.tpnd();

    bool capturing = m.capt();
    //uint8_t captureType = m.cptp();

    bool passed = m.epcp();

    Hash zFactor = ztk;

    if (capturing){
        uint8_t target = (endsquare + passed * ((toMove << 4) - 8));
        uint8_t captureType = m.cptp();
        pieces[captureType] ^= (1ULL << target);
        sides[!toMove] ^= (1ULL << target);

        zFactor = zpk[!toMove][captureType][target]; //when capturing, remove the target piece

        if (ev){
            int csb = mps[captureType][endsquare ^ (toMove * 56)];
            scores[!toMove] -= csb;
        }
    }
    
    sides[toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));
    pieces[typeMoved] ^= (1ULL << startsquare);
    pieces[typeEnded] ^= (1ULL << endsquare);

    zFactor ^= zpk[toMove][typeMoved][startsquare]; //replace zobrist keys of piece
    zFactor ^= zpk[toMove][typeEnded][endsquare];

    if (ev){
        int psb = mps[typeEnded][endsquare ^ (!toMove * 56)] - mps[typeMoved][startsquare ^ (!toMove * 56)];
        scores[toMove] += psb;
    }

    thm++;
    nodes++;

    //En Passant Square Update
    ep[thm] = m.dpsh() * (endsquare - 7 + (toMove << 4)) - 1;

    zFactor ^= (m.dpsh() * zek[endsquare & 7]); //file of en-passant square
    if (ep[thm - 1] != 255){
        zFactor ^= zek[ep[thm - 1] & 7]; // remove previous ep square
    }

    //chm[thm] = !(capturing or (typeMoved == 5)) * (chm[thm - 1] + 1);
    if (capturing or (typeMoved == 5)){
        chm[thm] = 0;
    } else {
        chm[thm] = chm[thm - 1] + 1;
    }

    //Adjust Castling Rights
    cr[thm] = cr[thm - 1]; //castling rights preserved at first

    if (cr[thm] != 0){
        uint8_t change = crc[startsquare] | crc[endsquare];
        if (change){
            cr[thm] &= ~change;
            zFactor ^= zck[cr[thm - 1]]; //if castling exists, xor old state
            zFactor ^= zck[cr[thm]]; //xor in new state
        }
    }

    //castling
    //still have to play Rh1-f1 or Ra1-d1
    uint8_t cat = m.cstl(); //valued 1 K or 2 Q
    if (cat){
        startsquare = cf[cat - 1] + 56 * toMove;
        endsquare = 7 - (cat << 1) + 56 * toMove;

        sides[toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));
        pieces[2] ^= ((1ULL << startsquare) | (1ULL << endsquare)); //move the rook

        zFactor ^= zpk[toMove][2][startsquare]; //move the rook from start and end
        zFactor ^= zpk[toMove][2][endsquare];

        if (ev){
            int psb = mps[2][endsquare ^ (56 * !toMove)] - mps[2][startsquare ^ (56 * !toMove)];
            scores[toMove] += psb;
        }
    }

    zhist[thm] = zhist[thm - 1] ^ zFactor;

    toMove ^= 1;   
}

template <bool ev> void Position::unmakeMove(Move m){
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

    if (ev){
        int psb = mps[typeEnded][endsquare ^ (toMove * 56)] - mps[typeMoved][startsquare ^ (toMove * 56)];
        scores[!toMove] -= psb;
    }
    
    if (capturing){
        uint8_t target = (endsquare + passed * ((!toMove << 4) - 8));
        uint8_t captureType = m.cptp();

        pieces[captureType] ^= (1ULL << target);
        sides[toMove] ^= (1ULL << target);

        if (ev){
            int csb = mps[captureType][endsquare ^ (!toMove * 56)];;
            scores[toMove] += csb;
        }
    }

    thm--;

    uint8_t cat = m.cstl();
    if (cat){
        startsquare = cf[cat - 1] + 56 * !toMove;
        endsquare = 7 - (cat << 1) + 56 * !toMove;

        sides[!toMove] ^= ((1ULL << startsquare) | (1ULL << endsquare));
        pieces[2] ^= ((1ULL << startsquare) | (1ULL << endsquare)); //move the rook

        if (ev){
            int psb = mps[2][endsquare ^ (56 * toMove)] - mps[2][startsquare ^ (56 * toMove)];
            scores[!toMove] -= psb;
        }
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
        makeMove<false>(moves[ply][i]);

        if (notValid(moves[ply][i])){
            unmakeMove<false>(moves[ply][i]);
            continue;
        }

        uint64_t next = perft(depth -1, ply + 1);

        unmakeMove<false>(moves[ply][i]);

        if (ply == 0){
            std::cout << moves[ply][i].toStr() << ": " << next << '\n';
        }
        

        pnodes += next;
    }

    if (ply == 0){
        std::cout << '\n' << pnodes << " nodes\n";
    }

    return pnodes;
}

void Position::sendMove(std::string str){
    int nc = generateMoves(0);

    for (int i = 0; i < nc; i++){
        if (moves[0][i].toStr() == str){
            //std::cout << "moves[0][i]: " << moves[0][i].info << '\n';
            makeMove<false>(moves[0][i]); //make without incremental eval

            //in parsing the tail, start over if there is a reset
            //note that this is special here. in perft, for example, this fails. 

            if (chm[thm] == 0){
                chm[0] = chm[thm];
                ep[0] = ep[thm];
                cr[0] = cr[thm];
                zhist[0] = zhist[thm];
                thm = 0;
            }
            return;
        }
    }
}

int Position::evaluate(){
    return scores[toMove] - scores[!toMove];
}

int Position::evaluateScratch(){
    scores[0] = 0;
    scores[1] = 0;

    Bitboard pcs;
    int f;

    for (int i = 0; i < 6; i++){
        pcs = sides[1] & pieces[i]; //white defined as original
        while (pcs){
            f = __builtin_ctzll(pcs);

            scores[1] += mps[i][f];

            pcs ^= (1ULL << f);
        }

        pcs = sides[0] & pieces[i];
        while (pcs){
            f = __builtin_ctzll(pcs);

            scores[0] += mps[i][f ^ 56];

            pcs ^= (1ULL << f);
        }
    }


    return scores[toMove] - scores[!toMove];
}










