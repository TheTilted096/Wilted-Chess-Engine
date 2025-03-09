/*
Class Definition of Position
Intermediate Object

Contains Static Evaluation Features

*/

#include "WiltedBitboards.cpp"

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

23: capture
24: promotion
25: en passant
26: castling
27: double push
*/

class Move{
    public:
        uint32_t info;

        Move(){ info = 0U; }
        Move(const uint32_t& m){ info = m; }
        Move(const Move& m){ info = m.info; }

        std::string toStr(){ //converts to algebraic (ex. a8a8)
            uint8_t start = gstsq();
            uint8_t end = gedsq();

            std::string result;

            result += ((start & 7) + 97);
            result += (8 - (start >> 3)) + 48;

            result += ((end & 7) + 97);
            result += (8 - (end >> 3)) + 48;

            if (gtpnd()){
                result += Bitboards::frchr[9 + gtpnd()];
            }

            return result;
        }

        int gstsq(){ return (info & 63U); } //start square (0 - 5), set upon construction
        void sstsq(int k){ info = k; } //WARNING
        
        int gedsq(){ return (info >> 6) & 63U; }  //end square (6 - 11)
        void sedsq(int k){ info |= (k << 6); }

        int gcptp(){ return (info >> 12) & 7U; } //capture type (12 - 15)
        void scptp(int k){  //set capture type, capture flag (23)
            info |= (k << 12); 
            info |= (1U << 23); //set capture flag
        }

        int gtpmv(){ return (info >> 15) & 7U; } //type moved (15 - 18)
        void stpmv(int k){ info |= (k << 15); }
    
        int gtpnd(){ return (info >> 18) & 7U; } //type ended (18 - 20)
        void stpnd(int k){  //tpnd only set upon promotion
            info |= (k << 18); 
            info |= (1U << 24);
        }

        int gcstl(){ return (info >> 21) & 3U; } //castling (21-22)
        void scstl(int k){  //set castling flag (26)
            info |= (k << 21); 
            info |= (1U << 26);
        }

        //bool gprmt(){ return (info >> 24) & 1U; } //seen by setting typeMoved
        //void sprmt(){ info |= (1U << 24); }

        bool gepcp(){ return (info >> 25) & 1U; } //en passant (25) set when pieceAt finds nothing
        //void sepcp(){ info |= (1U << 25); } set upon deduction of capture type

        bool gdpsh(){ return (info >> 27); } // double push (27)
        void sdpsh(){ info |= (1U << 27); }

        int state(){ return (info >> 23); }

        void show(){
            std::cout << toStr() << ' ' << info << ' ' << gstsq() << ' ' << gedsq() << ' ' << gcptp() << ' ' << 
                gtpmv() << ' ' << gtpnd() << ' ' << gcstl() << ' ' << state() << '\n';
        }

        bool operator==(const Move& m){
            return info == m.info;
        }
};

class Position : public Bitboards{
    public:
        Move moves[96][128];
        int mprior[96][128];

        uint64_t nodes;

        int scores[2], eScores[2];
        int gamePhase;

        //Bitboard atktbl[2][5]; //atktbl[s][i] is the threats to piece i posed by s. i.e. atktbl[0][4] = black threats to knights (black pawns)

        static constexpr int phases[6] = {0, 14, 8, 5, 4, 2};
        static constexpr int totalPhase = 128;

        //Original
        /*        
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

        int eps[6][64] =
        {{25, 7, -33, -15, 30, 28, 38, 8, 
        -33, -12, -22, -20, -40, 31, 8, -14, 
        38, 34, 20, -19, -14, 39, -2, 18, 
        -35, -26, 1, -21, -37, -29, 14, -7, 
        37, 24, -23, -3, -6, -28, 34, -24, 
        23, -12, -10, -14, 23, 1, -10, -40, 
        11, 10, 9, -38, 13, -13, 14, -12, 
        -30, -2, -27, -38, -2, 26, -17, 9},

        {-78, -69, 60, -42, 9, -52, 19, -37, 
        -43, 0, -79, 34, 89, 76, 29, -17, 
        -23, 64, 71, 3, -2, 55, 5, -21, 
        13, -41, -12, -8, 68, -82, 55, -38, 
        -64, -68, 20, -29, -24, 43, -85, -57, 
        81, 81, 8, -26, -73, 81, 60, 25, 
        -14, -51, 22, 21, 33, 77, 45, 44, 
        11, 12, -58, -71, -6, 35, -28, -14},

        {-28, 14, 36, -40, 4, 31, -10, -38, 
        -22, 36, 39, -50, 11, 21, 9, 41, 
        -21, 45, 0, 9, 7, -11, -38, 8, 
        1, -33, 1, -13, 37, 15, 39, 32, 
        19, -24, -38, -25, 19, -23, -30, 34, 
        28, -43, -20, 7, -32, -44, -16, 19, 
        46, 47, 21, -43, 49, -49, 47, -11, 
        16, -24, 21, -3, -10, 31, 48, -1},

        {14, -29, 33, 28, 10, 25, 26, -26, 
        15, 14, -4, 28, 13, -1, -26, -15, 
        -1, -23, -18, 22, -7, -30, -26, -2, 
        11, 27, -35, 34, 22, -17, -26, 34, 
        6, 4, 9, -19, -12, -17, -2, -18, 
        -30, -2, 33, 31, 2, -21, 33, 25, 
        5, -11, -9, -34, -14, 12, 19, 18, 
        -17, 7, -15, -24, 26, -28, 25, -5},

        {23, 4, -16, 1, 5, 10, 21, 0, 
        21, -17, 23, 28, 24, 9, 1, 15, 
        -17, -28, -26, 21, -13, 22, 25, 29, 
        -11, -1, -17, 2, -14, -5, 27, 13, 
        0, -1, -1, -12, 14, 16, 13, -10, 
        21, -2, -12, -20, 24, 22, -8, -20, 
        -16, -18, 3, 19, 19, 13, -22, -3, 
        26, -7, -1, -17, -23, -12, -24, 20},

        {0, 0, 0, 0, 0, 0, 0, 0,
        -7, -9, 2, -3, -7, -6, -7, -9, 
        -8, -3, -1, -3, -5, 1, -9, 4, 
        4, -6, 9, -6, 6, 8, 8, 1, 
        2, 3, 5, 1, 3, -6, 1, 4, 
        -6, 2, -7, -9, -9, 9, 4, 3, 
        -1, -4, -9, -2, -6, 6, -8, 8, 
        0, 0, 0, 0, 0, 0, 0, 0,}};

        int material[6] = {0, 900, 500, 350, 300, 100};
        */

        double mps[6][64] = 
        {{5, 20, 0, 30, 23, 9, 18, 7, 
        -23, -30, 11, -2, 39, 23, 9, -31, 
        -35, -13, -22, -38, 5, -5, 21, -6, 
        36, -6, 29, 4, -23, -11, -9, 25, 
        -40, 21, 13, 33, -38, -25, -19, -6, 
        0, 1, -4, -10, -3, -4, -28, -5, 
        -15, 36, 8, -9, -3, -24, 24, 43, 
        -26, 0, 21, 8, -3, -10, 20, -31},

        {-77, -26, 48, -63, 32, 67, -42, -38, 
        -88, -34, 5, 4, -86, -7, -45, 63, 
        41, -43, -16, 23, -20, -40, -87, 86, 
        45, 79, 38, -76, 64, -62, -47, 81, 
        49, -27, -59, 44, -5, -86, 46, -74, 
        44, 26, 47, -42, 80, 29, 0, -17, 
        24, 54, 49, 38, -67, -9, 36, -26, 
        8, -60, 40, 64, 65, -46, 31, 26},

        {-42, -17, 33, -6, -6, 18, 30, -33, 
        -4, -41, 46, -31, 35, 0, -8, -31, 
        -22, 20, 0, 37, 33, 38, -37, 19, 
        41, -3, 34, 24, 36, -24, 34, 42, 
        30, 24, 13, -38, 16, -3, 41, 16, 
        -17, -41, -30, -9, 28, 33, -13, 8, 
        1, -2, -40, -1, -10, -33, -16, -8, 
        2, 11, -36, 3, 19, 16, -3, -14},

        {-9, -33, 18, 28, -19, -29, -24, 24, 
        11, 13, -28, -30, 10, -34, 14, 15, 
        -20, -3, -31, 2, 25, -14, 18, 2, 
        18, -28, 6, -25, 6, -13, 0, 6, 
        20, 28, 9, -14, 1, 14, -13, 17, 
        2, 7, 17, -1, 2, 6, -26, -10, 
        -2, 6, 1, -11, 16, -32, -14, 1, 
        -30, 19, 1, -17, -5, -9, 1, 9},

        {21, 19, 8, -8, 24, 17, -6, 14, 
        -8, 20, 19, 23, -8, 26, 24, 18, 
        -7, 31, -26, -12, -7, -7, 21, 12, 
        0, 26, 14, 23, 30, -8, 28, 0, 
        17, -20, -5, -1, -3, 29, 4, 2, 
        -11, 17, 20, 19, -17, 12, -7, -3, 
        8, 21, 5, -15, 27, 19, -7, -5, 
        8, -25, 5, -19, -7, -25, 0, 11},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        4, -4, -6, -8, 4, -5, 0, -6, 
        1, 7, 3, 11, 2, 10, -7, 9, 
        -2, 12, -8, 5, 0, -6, 1, -10, 
        -13, -12, -2, 3, -4, -3, 0, -15, 
        -19, -8, -7, 4, 7, -10, 0, -11, 
        -30, -30, -5, -21, 1, -5, -2, -18, 
        0, 0, 0, 0, 0, 0, 0, 0}};


        double eps[6][64] = 
        {{24, 6, -32, -15, 28, 25, 31, 7, 
        -32, -10, -20, -19, -38, 26, 8, -13, 
        29, 30, 21, -16, -11, 35, 0, 17, 
        -33, -21, 4, -17, -32, -23, 15, -7, 
        29, 20, -20, 0, -1, -19, 27, -24, 
        20, -10, -7, -9, 26, 0, -6, -39, 
        9, 4, 6, -33, 9, -4, 7, -10, 
        -29, -3, -27, -38, -3, 12, -14, 7},

        {-77, -68, 59, -41, 9, -51, 19, -36, 
        -42, 0, -78, 34, 89, 76, 29, -17, 
        -23, 64, 70, 3, -1, 55, 5, -21, 
        12, -41, -11, -6, 66, -80, 55, -38, 
        -64, -66, 20, -28, -22, 43, -84, -54, 
        72, 80, 7, -24, -72, 80, 60, 25, 
        -13, -50, 21, 20, 33, 76, 44, 44, 
        11, 12, -57, -70, -7, 35, -27, -13},

        {-25, 15, 36, -38, 5, 31, -8, -34, 
        -17, 38, 39, -46, 12, 22, 11, 42, 
        -15, 44, 2, 11, 9, -8, -32, 9, 
        2, -28, 3, -11, 34, 16, 35, 29, 
        18, -22, -35, -23, 17, -21, -29, 30, 
        23, -41, -18, 7, -31, -44, -14, 17, 
        18, 40, 22, -39, 45, -44, 40, -14, 
        0, -20, 23, -2, -7, 26, 29, 0},

        {13, -28, 29, 25, 10, 24, 25, -26, 
        14, 12, -4, 27, 12, 0, -25, -15, 
        0, -22, -17, 23, -6, -28, -25, -2, 
        7, 27, -32, 35, 21, -15, -24, 31, 
        3, 2, 10, -15, -9, -15, -2, -18, 
        -27, -2, 22, 30, 1, -18, 30, 16, 
        4, -9, -9, -32, -11, 10, 20, 16, 
        -17, 4, -30, -24, 16, -26, 22, -9},

        {19, 3, -16, 0, 3, 9, 20, 0, 
        20, -16, 22, 26, 24, 8, 0, 11, 
        -16, -25, -23, 24, -9, 23, 24, 25, 
        -10, 1, -13, 8, -9, -2, 23, 11, 
        0, 0, 3, -4, 16, 18, 11, -9, 
        17, -7, -2, -17, 23, 22, -8, -19, 
        -16, -18, 0, 19, 18, 7, -22, -3, 
        6, -22, -3, -17, -22, -12, -25, 11},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        4, -2, 7, 1, -3, 0, 0, -1, 
        4, 7, 4, 6, 0, 8, 0, 13, 
        22, 7, 8, 5, 10, 18, 15, 11, 
        6, 9, 9, 1, 5, -1, -2, 8, 
        -8, 2, -2, -7, -5, 4, 2, -9, 
        0, -13, -7, -8, -11, 0, -9, -3, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        double material[6] = {0, 918, 459, 310, 270, 107};
        

        /*
        static constexpr int midx[5] = {0, 9, 37, 52, 66};

        int mmb[75] = 
        {-6, 25, -18, 11, -1, -8, -28, -19, -38,
        -57, 25, 31, -40, -30, -67, -80, 43, -85, 39, 60, -21, -62, 64, 14, -37, -55, 45, 20, -43, 24, 32, 88, 68, 71, -17, -25, -56,
        42, 10, -30, -43, 7, 4, 28, -44, -46, 30, 50, -25, -28, -40, 6,
        -23, 33, -27, -9, 19, -30, -29, -4, -23, -8, 13, -17, 17, 16,
        -29, -16, 22, 10, -18, 20, -2, 25, 8};

        int emb[75] = 
        {-29, -30, 22, 26, -28, -7, -15, 16, 24,
        8, -27, -66, 60, -80, -64, 5, 84, -9, 87, -54, 59, 11, 17, -57, 75, -29, 30, 19, 21, 80, 55, -56, -47, -34, -52, 68, -60,
        -50, -47, -50, -22, 13, -33, -6, -13, -5, -35, 24, 20, 37, 50, 43,
        16, -16, 19, -22, -28, -35, 35, -25, 8, -33, 22, -32, -26, -27,
        24, 2, -18, -12, -10, -21, 20, 1, -19};
        */


        Position();
        bool isAttacked(int, bool); //checks if a square is attacked by a side
        bool isChecked(bool); //if a side is in check
        bool notValid(Move); //if past player left king check

        int evaluate(); //returns static eval
        int evaluateScratch(); //computes all psqts and mobility from scratch, then evaluates

        uint64_t perft(int, int);
        int countLegal();

        template <bool, int, bool> void __impl_makeMove(Move); //side, movestate, incremental eval
        template <bool, int, bool> void __impl_unmakeMove(Move);
        
        template <bool> void makeMove(Move);
        template <bool> void unmakeMove(Move);

        template <bool, int, bool> int __impl_generateMoves(int); //side, castling, captures
        template <bool> int __swit_generateMoves(int);

        int generateMoves(int);
        int generateCaptures(int);

        void passMove();
        void unpassMove();

        void sendMove(std::string);

};

Position::Position(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            mps[i][j] += material[i];
            eps[i][j] += material[i];
        }
    }
    nodes = 0ULL;
}

bool Position::isAttacked(int sq, bool s){
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
    int cat = m.gcstl(); //Q = 2, K = 1
    if (cat){
        //std::cout << "Castle: " << (int) cat << '\n';
        Bitboard cmask = cmk[cat - 1] << (!toMove * 56);
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

int Position::evaluate(){
    //return ((scores[toMove] - scores[!toMove]) * gamePhase + (eScores[toMove] - eScores[!toMove]) * (totalPhase - gamePhase)) / totalPhase;
    //return scores[toMove] - scores[!toMove];

    int m = (scores[toMove] - scores[!toMove]) * gamePhase;
    int e = (eScores[toMove] - eScores[!toMove]) * (totalPhase - gamePhase);

    return (m + e) / totalPhase;
}

int Position::evaluateScratch(){
    scores[0] = 0;
    scores[1] = 0;
    eScores[0] = 0;
    eScores[1] = 0;

    gamePhase = 0;

    Bitboard pcs;//, mvst;
    int f;//, m;

    for (int i = 0; i < 6; i++){ //computing piece-square bonuses
        pcs = sides[1] & pieces[i];
        while (pcs){
            f = __builtin_ctzll(pcs);

            scores[1] += mps[i][f]; //black is defined as the side that 'flips'
            eScores[1] += eps[i][f];

            pcs ^= (1ULL << f);
        }

        pcs = sides[0] & pieces[i];
        while (pcs){
            f = __builtin_ctzll(pcs);

            scores[0] += mps[i][f ^ 56];
            eScores[0] += eps[i][f ^ 56];

            pcs ^= (1ULL << f);
        }
    }

    for (int j = 1; j < 6; j++){
        gamePhase += phases[j] * __builtin_popcountll(pieces[j]);
    }

    /* Scratch Mobility Calculations
    atktbl[0][4] = (sides[0] & pieces[5] & 0x7F7F7F7F7F7F7F7FULL) << 9;
    atktbl[0][4] |= (sides[0] & pieces[5] & 0xFEFEFEFEFEFEFEFEULL) << 7;

    atktbl[1][4] = (sides[1] & pieces[5] & 0x7F7F7F7F7F7F7F7FULL) >> 7;
    atktbl[1][4] |= (sides[1] & pieces[5] & 0xFEFEFEFEFEFEFEFEULL) >> 9;

    Bitboard occ = sides[0] | sides[1];

    atktbl[0][3] = atktbl[0][4]; //knights don't threaten bishops
    atktbl[0][2] = atktbl[0][3]; //carry over attacks to rooks

    atktbl[1][3] = atktbl[1][4];
    atktbl[1][2] = atktbl[1][3];

    for (int s = 0; s < 2; s++){ //add knight and bishop to rook threats; calculate knight and bishop mobs;
        pcs = sides[s] & pieces[4]; //knights
        while (pcs){
            f = __builtin_ctzll(pcs);

            mvst = KnightAttacks[f];
            atktbl[s][2] |= mvst;
            m = __builtin_popcountll(mvst & ~atktbl[!s][4]);

            scores[s] += mmb[midx[4] + m];
            eScores[s] += emb[midx[4] + m];

            pcs ^= (1ULL << f);
        }

        pcs = sides[s] & pieces[3]; //bishops
        while (pcs){
            f = __builtin_ctzll(pcs);

            mvst = bishopAttack(f, occ);
            atktbl[s][2] |= mvst;
            m = __builtin_popcountll(mvst & ~atktbl[!s][3]);

            scores[s] += mmb[midx[3] + m];
            eScores[s] += emb[midx[3] + m];

            pcs ^= (1ULL << f);            
        }
    }

    atktbl[0][1] = atktbl[0][2]; //carry rook threats to queen
    atktbl[1][1] = atktbl[1][2];

    for (int s = 0; s < 2; s++){ //add rook moves to queen threats; calculate rook mobs
        pcs = sides[s] & pieces[2];
        while (pcs){
            f = __builtin_ctzll(pcs);

            mvst = rookAttack(f, occ);
            atktbl[s][1] |= mvst;

            m = __builtin_popcountll(mvst & ~atktbl[!s][2]);
            scores[s] += mmb[midx[2] + m];
            eScores[s] += emb[midx[2] + m];

            pcs ^= (1ULL << f);
        }
    }

    atktbl[0][0] = atktbl[0][1]; //carry queen threats to kings
    atktbl[1][0] = atktbl[1][1];

    for (int s = 0; s < 2; s++){ //add queen attacks to king; calculate queen mobs
        pcs = sides[s] & pieces[1];
        while (pcs){
            f = __builtin_ctzll(pcs);

            mvst = (rookAttack(f, occ) | bishopAttack(f, occ));
            atktbl[s][0] |= mvst;

            m = __builtin_popcountll(mvst & ~atktbl[!s][1]);
            scores[s] = mmb[midx[1] + m];
            eScores[s] = emb[midx[1] + m];

            pcs ^= (1ULL << f);
        }
    }
    
    atktbl[0][0] |= KingAttacks[__builtin_ctzll(sides[0] & pieces[0])]; //black king threatens white king
    
    mvst = KingAttacks[__builtin_ctzll(sides[1] & pieces[0])]; //white king attacks
    atktbl[1][0] |= mvst; //white king threatens black king
    m = __builtin_popcountll(mvst & ~atktbl[0][0]); //calculate white king mobility and add
    scores[1] += mmb[midx[0] + m];
    eScores[1] += emb[midx[0] + m];

    mvst = KingAttacks[__builtin_ctzll(sides[0] & pieces[0])]; //calculate black king mobility and add
    m = __builtin_popcountll(mvst & ~atktbl[1][0]);
    scores[0] += mmb[midx[0] + m];
    eScores[0] += emb[midx[0] + m];
    */

    return evaluate();
}

template <bool who, int cstl, bool cpex> int Position::__impl_generateMoves(int ply){
    int tgm = 0;

    Bitboard mvst, pcs, xset;
    Bitboard occ = sides[0] | sides[1];

    Bitboard ours = sides[who];
    Bitboard opps = sides[!who];

    int f, p, cc;

    //Pawns:
    pcs = pieces[5] & ours;
    
    Bitboard pawnopps = opps;
    if (ep[thm] != 255){
        pawnopps |= (1ULL << ep[thm]);
    }

    Bitboard leftcap = (pcs & 0xFEFEFEFEFEFEFEFEULL) << 7;
	Bitboard rightcap = (pcs & 0x7F7F7F7F7F7F7F7FULL) << 9;

    if constexpr (who){
        leftcap >>= (who << 4);
        rightcap >>= (who << 4);
    }

    leftcap &= pawnopps;
    rightcap &= pawnopps;

    while (rightcap){
        f = __builtin_ctzll(rightcap);

        moves[ply][tgm].sstsq(f - 9 + (who << 4));
        moves[ply][tgm].sedsq(f);

        cc = pieceAt(f);
        moves[ply][tgm].scptp(cc);

        moves[ply][tgm].stpmv(5);

        bool prmt = ((1ULL << f) & 0xFF000000000000FF); //cast promotion mask to bool
        if (prmt){
            Move temp = moves[ply][tgm];

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(1);
            tgm++;

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(2);
            tgm++;

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(3);
            tgm++;

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(4);     
        }

        tgm++;
        rightcap ^= (1ULL << f);
    }

    while (leftcap){
        f = __builtin_ctzll(leftcap);

        moves[ply][tgm].sstsq(f - 7 + (who << 4));
        moves[ply][tgm].sedsq(f);

        cc = pieceAt(f);
        moves[ply][tgm].scptp(cc);

        moves[ply][tgm].stpmv(5);

        bool prmt = ((1ULL << f) & 0xFF000000000000FF); //cast promotion mask to bool
        if (prmt){
            Move temp = moves[ply][tgm];

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(1);
            tgm++;

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(2);
            tgm++;

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(3);
            tgm++;

            moves[ply][tgm] = temp;
            moves[ply][tgm].stpnd(4);     
        }

        tgm++;
        leftcap ^= (1ULL << f);
    }

    if constexpr (!cpex){
        Bitboard pshtrgt = ((pcs << 8) >> (who << 4)) & ~occ;
        Bitboard dpshtrgt = ((pshtrgt << 8) >> (who << 4)) & ~occ & (0xFF000000ULL << (who << 3));

        while (pshtrgt){
            f = __builtin_ctzll(pshtrgt);
            moves[ply][tgm].sstsq(f - 8 + (who << 4));
            moves[ply][tgm].sedsq(f);

            moves[ply][tgm].stpmv(5);

            bool prmt = ((1ULL << f) & 0xFF000000000000FF); //cast promotion mask to bool
            if (prmt){
                Move temp = moves[ply][tgm];
    
                moves[ply][tgm] = temp;
                moves[ply][tgm].stpnd(1);
                tgm++;
    
                moves[ply][tgm] = temp;
                moves[ply][tgm].stpnd(2);
                tgm++;
    
                moves[ply][tgm] = temp;
                moves[ply][tgm].stpnd(3);
                tgm++;
    
                moves[ply][tgm] = temp;
                moves[ply][tgm].stpnd(4);     
            }

            tgm++;
            pshtrgt ^= (1ULL << f);
        }
    
        while (dpshtrgt){
            f = __builtin_ctzll(dpshtrgt);

            moves[ply][tgm].sstsq(f - 16 + (who << 5));
            moves[ply][tgm].sedsq(f);
            moves[ply][tgm].stpmv(5);

            moves[ply][tgm].sdpsh();

            dpshtrgt ^= (1ULL << f);
            tgm++;
        }
    
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

            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(p);

            cc = pieceAt(p);
            moves[ply][tgm].scptp(cc);

            moves[ply][tgm].stpmv(4);

            tgm++;
            xset ^= (1ULL << p);
        }

        if constexpr (!cpex){
            while (mvst){
                p = __builtin_ctzll(mvst);

                moves[ply][tgm].sstsq(f);
                moves[ply][tgm].sedsq(p);

                moves[ply][tgm].stpmv(4);

                tgm++;
                mvst ^= (1ULL << p);
            }

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

            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(p);

            cc = pieceAt(p);
            moves[ply][tgm].scptp(cc);

            moves[ply][tgm].stpmv(3);

            tgm++;
            xset ^= (1ULL << p);
        }

        if constexpr (!cpex){
            while (mvst){
                p = __builtin_ctzll(mvst);

                moves[ply][tgm].sstsq(f);
                moves[ply][tgm].sedsq(p);

                moves[ply][tgm].stpmv(3);

                tgm++;
                mvst ^= (1ULL << p);
            }

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

            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(p);

            cc = pieceAt(p);
            moves[ply][tgm].scptp(cc);

            moves[ply][tgm].stpmv(2);

            tgm++;
            xset ^= (1ULL << p);
        }

        if constexpr (!cpex){
            while (mvst){
                p = __builtin_ctzll(mvst);

                moves[ply][tgm].sstsq(f);
                moves[ply][tgm].sedsq(p);

                moves[ply][tgm].stpmv(2);

                tgm++;
                mvst ^= (1ULL << p);
            }

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

            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(p);

            cc = pieceAt(p);
            moves[ply][tgm].scptp(cc);

            moves[ply][tgm].stpmv(1);

            tgm++;
            xset ^= (1ULL << p);
        }

        if constexpr (!cpex){
            while (mvst){
                p = __builtin_ctzll(mvst);

                moves[ply][tgm].sstsq(f);
                moves[ply][tgm].sedsq(p);

                moves[ply][tgm].stpmv(1);

                tgm++;
                mvst ^= (1ULL << p);
            }

        }

        pcs ^= (1ULL << f);
    }

    //Kings
    f = __builtin_ctzll(pieces[0] & ours);

    mvst = KingAttacks[f] & ~ours;
    xset = mvst & opps;
    mvst ^= xset;

    while (xset){
        p = __builtin_ctzll(xset);

        moves[ply][tgm].sstsq(f);
        moves[ply][tgm].sedsq(p);

        cc = pieceAt(p);
        moves[ply][tgm].scptp(cc);

        //moves[ply][tgm].stpmv(0);

        tgm++;
        xset ^= (1ULL << p);
    }

    if constexpr (!cpex){
        while (mvst){
            p = __builtin_ctzll(mvst);

            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(p);

            //moves[ply][tgm].stpmv(0);

            tgm++;
            mvst ^= (1ULL << p);
        }
    }

    //King Castling

    //Kingside
    if constexpr (cstl & 1){
        if (!((coc[0] << (who * 56)) & occ)){
            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(6 + (56 * who));
            moves[ply][tgm].scstl(1);
            tgm++;
        }
    }

    //Queenside
    if constexpr (cstl & 2){
        if (!((coc[1] << (who * 56)) & occ)){
            moves[ply][tgm].sstsq(f);
            moves[ply][tgm].sedsq(2 + (56 * who));
            moves[ply][tgm].scstl(2);
            tgm++;
        }
    }

    return tgm;
}

template <bool cpex> inline int Position::__swit_generateMoves(int ply){
    int ind = (toMove << 2) ^ ((cr[thm] >> (!toMove << 1)) & 0x3);

    switch (ind){
        case 0:
            return __impl_generateMoves<false, 0, cpex>(ply);
        case 1:
            return __impl_generateMoves<false, 1, cpex>(ply);
        case 2:
            return __impl_generateMoves<false, 2, cpex>(ply);
        case 3:
            return __impl_generateMoves<false, 3, cpex>(ply);
        case 4:
            return __impl_generateMoves<true, 0, cpex>(ply);
        case 5:
            return __impl_generateMoves<true, 1, cpex>(ply);
        case 6:
            return __impl_generateMoves<true, 2, cpex>(ply);
        case 7:
            return __impl_generateMoves<true, 3, cpex>(ply);
        default:
            __builtin_unreachable();        
    }
}

inline int Position::generateMoves(int ply){ return __swit_generateMoves<false>(ply); }

inline int Position::generateCaptures(int ply){ return __swit_generateMoves<true>(ply); }

template <bool who, int state, bool eval> void Position::__impl_makeMove(Move m){
    uint8_t start = m.gstsq();
    uint8_t end = m.gedsq();
    uint8_t typei = m.gtpmv();
    uint8_t typef;

    constexpr bool capt = state & 1;

    if constexpr (state & 2){ //promotion
        typef = m.gtpnd();
    } else {
        typef = typei;
    }

    Hash zFactor = ztk;

    if constexpr (capt){
        constexpr bool pass = state & 4;
        uint8_t target = (end + pass * ((who << 4) - 8));
        uint8_t ctype = m.gcptp();

        pieces[ctype] ^= (1ULL << target);
        sides[!who] ^= (1ULL << target);

        zFactor ^= zpk[!who][ctype][target];

        if constexpr (eval){
            scores[!who] -= mps[ctype][target ^ (56 * who)];
            eScores[!who] -= eps[ctype][target ^ (56 * who)];
            gamePhase -= phases[ctype];
        }
    }

    sides[who] ^= ((1ULL << start) | (1ULL << end));
    pieces[typei] ^= (1ULL << start);
    pieces[typef] ^= (1ULL << end);

    zFactor ^= zpk[who][typei][start]; //replace zobrist keys of piece
    zFactor ^= zpk[who][typef][end];

    if constexpr (eval){
        scores[who] += (mps[typef][end ^ (56 * !who)] - mps[typei][start ^ (56 * !who)]);
        eScores[who] += (eps[typef][end ^ (56 * !who)] - eps[typei][start ^ (56 * !who)]);
        if constexpr (state & 2){ //promotion
            gamePhase -= phases[5];
            gamePhase += phases[typef];
        }
    }

    thm++;
    nodes++;

    if constexpr (state & 16){ //double-push
        ep[thm] = end - 8 + (who << 4);
        zFactor ^= zek[end & 7];
    } else {
        ep[thm] = 255;
    }

    if (ep[thm - 1] != 255){
        zFactor ^= zek[ep[thm - 1] & 7];
    }

    if constexpr (capt){
        chm[thm] = 0;
    } else if (typei == 5){
        chm[thm] = 0;
    } else {
        chm[thm] = chm[thm - 1] + 1;
    }

    cr[thm] = cr[thm - 1];
    if (cr[thm] != 0){
        uint8_t change = crc[start] | crc[end];
        if (change){
            cr[thm] &= ~change;
            zFactor ^= zck[cr[thm - 1]];
            zFactor ^= zck[cr[thm]];
        }
    }

    if constexpr (state & 8){ //castling
        uint8_t cat = m.gcstl();
        start = cf[cat - 1] ^ (56 * who);
        end = 7 - (cat << 1) + 56 * who;

        sides[who] ^= ((1ULL << start) | (1ULL << end));
        pieces[2] ^= ((1ULL << start) | (1ULL << end));

        zFactor ^= zpk[who][2][start]; //move the rook from start and end
        zFactor ^= zpk[who][2][end];

        if constexpr (eval){
            scores[who] += (mps[2][end ^ (56 * !who)] - mps[2][start ^ (56 * !who)]);
            eScores[who] += (eps[2][end ^ (56 * !who)] - eps[2][start ^ (56 * !who)]);
        }
    }

    zhist[thm] = zhist[thm - 1] ^ zFactor;
    toMove ^= 1;
}

template <bool who, int state, bool eval> void Position::__impl_unmakeMove(Move m){
    uint8_t start = m.gstsq();
    uint8_t end = m.gedsq();
    uint8_t typei = m.gtpmv();

    uint8_t typef;
    constexpr bool capt = state & 1;

    if constexpr (state & 2){
        typef = m.gtpnd();
    } else {
        typef = typei;
    }

    sides[!who] ^= ((1ULL << start) | (1ULL << end));
    pieces[typei] ^= (1ULL << start);
    pieces[typef] ^= (1ULL << end);

    if constexpr (eval){
        scores[!who] -= (mps[typef][end ^ (56 * who)] - mps[typei][start ^ (56 * who)]);
        eScores[!who] -= (eps[typef][end ^ (56 * who)] - eps[typei][start ^ (56 * who)]);
        if constexpr (state & 2){
            gamePhase += phases[5];
            gamePhase -= phases[typef];
        }
    }

    if constexpr (capt){
        constexpr bool pass = state & 4;
        uint8_t target = end + pass * ((!who << 4) - 8);
        uint8_t ctype = m.gcptp();

        pieces[ctype] ^= (1ULL << target);
        sides[who] ^= (1ULL << target);

        if constexpr (eval){
            scores[who] += mps[ctype][target ^ (56 * !who)];
            eScores[who] += eps[ctype][target ^ (56 * !who)];
            gamePhase += phases[ctype];
        }
    }

    thm--;

    if constexpr (state & 8){
        uint8_t cat = m.gcstl();
        start = cf[cat - 1] ^ (56 * !who);
        end = 7 - (cat << 1) + (56 * !who);

        sides[!who] ^= ((1ULL << start) | (1ULL << end));
        pieces[2] ^= ((1ULL << start) | (1ULL << end));

        if constexpr (eval){
            scores[!who] -= (mps[2][end ^ (56 * who)] - mps[2][start ^ (56 * who)]);
            eScores[!who] -= (eps[2][end ^ (56 * who)] - eps[2][start ^ (56 * who)]);
        }
    }

    toMove ^= 1;
}

template <bool eval> inline void Position::makeMove(Move m){
    int ind = (toMove << 5) ^ m.state();

    switch (ind){
        case 0:
            __impl_makeMove<false, 0, eval>(m);
            break;
        case 1:
            __impl_makeMove<false, 1, eval>(m);
            break;
        case 2:
            __impl_makeMove<false, 2, eval>(m);
            break;
        case 3:
            __impl_makeMove<false, 3, eval>(m);
            break;
        case 5:
            __impl_makeMove<false, 5, eval>(m);
            break;
        case 8:
            __impl_makeMove<false, 8, eval>(m);
            break;
        case 16:
            __impl_makeMove<false, 16, eval>(m);
            break;

        case 32:
            __impl_makeMove<true, 0, eval>(m);
            break;
        case 33:
            __impl_makeMove<true, 1, eval>(m);
            break;
        case 34:
            __impl_makeMove<true, 2, eval>(m);
            break;
        case 35:
            __impl_makeMove<true, 3, eval>(m);
            break;
        case 37:
            __impl_makeMove<true, 5, eval>(m);
            break;
        case 40:
            __impl_makeMove<true, 8, eval>(m);
            break;
        case 48:
            __impl_makeMove<true, 16, eval>(m);
            break;
        
        default:
            //exit(1);
            __builtin_unreachable();
    }
}

template <bool eval> inline void Position::unmakeMove(Move m){
    int ind = (toMove << 5) ^ m.state();

    switch (ind){
        case 0:
            __impl_unmakeMove<false, 0, eval>(m);
            break;
        case 1:
            __impl_unmakeMove<false, 1, eval>(m);
            break;
        case 2:
            __impl_unmakeMove<false, 2, eval>(m);
            break;
        case 3:
            __impl_unmakeMove<false, 3, eval>(m);
            break;
        case 5:
            __impl_unmakeMove<false, 5, eval>(m);
            break;
        case 8:
            __impl_unmakeMove<false, 8, eval>(m);
            break;
        case 16:
            __impl_unmakeMove<false, 16, eval>(m);
            break;

        case 32:
            __impl_unmakeMove<true, 0, eval>(m);
            break;
        case 33:
            __impl_unmakeMove<true, 1, eval>(m);
            break;
        case 34:
            __impl_unmakeMove<true, 2, eval>(m);
            break;
        case 35:
            __impl_unmakeMove<true, 3, eval>(m);
            break;
        case 37:
            __impl_unmakeMove<true, 5, eval>(m);
            break;
        case 40:
            __impl_unmakeMove<true, 8, eval>(m);
            break;
        case 48:
            __impl_unmakeMove<true, 16, eval>(m);
            break;
        
        default:
            __builtin_unreachable();
    }
}

uint64_t Position::perft(int depth, int ply){
    uint64_t pnodes = 0;
    if (depth == 0){
        return 1ULL;
    }

    int v = generateMoves(ply);

    for (int i = 0; i < v; i++){
        makeMove<false>(moves[ply][i]);
        //std::cout << "make " << moves[ply][i].toStr() << '\n';

        if (notValid(moves[ply][i])){
            unmakeMove<false>(moves[ply][i]);
            //std::cout << "Legality\n";
            continue;
        }

        uint64_t next = perft(depth - 1, ply + 1);

        unmakeMove<false>(moves[ply][i]);

        if (ply == 0){
            //print();
            std::cout << moves[ply][i].toStr() << ": " << next << '\n';
        }

        //std::cout << "unmake " << moves[ply][i].toStr() << '\n';

        pnodes += next;
    }

    if (ply == 0){
        std::cout << '\n' << pnodes << " nodes\n";
    }

    return pnodes;
}

int Position::countLegal(){
    int l = generateMoves(63);
    int r = 0;

    for (int i = 0; i < l; i++){
        makeMove<false>(moves[63][i]);

        if (notValid(moves[63][i])){
            unmakeMove<false>(moves[63][i]);
            continue;
        }

        unmakeMove<false>(moves[63][i]);
        r++;
    }

    return r;
}

void Position::passMove(){
    toMove ^= 1;
    //nodes++; //count as a node?
    thm++;
    zhist[thm] = zhist[thm - 1] ^ ztk;
    chm[thm] = chm[thm - 1];
    ep[thm] = 255; //after passing, remove ep square
    cr[thm] = cr[thm - 1]; //retain castling rights.
}

void Position::unpassMove(){
    toMove ^= 1;
    thm--;
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














