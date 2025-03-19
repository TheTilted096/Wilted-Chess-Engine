/*

Autoplay Program for the Wilted Engine

TheTilted096, 3-5-25

*/

/*

(R - sig(S))^2

2 (R - sig(S)) * (-sig'(S)) * S'

S' = wi - bi (phase const mid/end)

S' = cwk - cbk

*/

#include "WiltedEngine.cpp"

class Guesser : public Bitboards{
    public:
        double scores[2], eScores[2];
        double gamePhase;

        static constexpr double phases[6] = {0, 14, 8, 5, 4, 2};
        static constexpr double totalPhase = 128;

        double mps[6][64] = 
        {{4, 19, 0, 29, 21, 8, 17, 6, 
        -22, -29, 11, -1, 39, 21, 9, -30, 
        -35, -13, -21, -36, 5, -5, 22, -5, 
        36, -4, 29, 5, -20, -8, -7, 24, 
        -41, 19, 13, 32, -35, -19, -20, -5, 
        0, 2, -3, -8, -4, -3, -24, -2, 
        -16, 33, 12, -10, -10, -16, 11, 29, 
        -26, 0, 17, -4, 8, -27, 42, -26},

        {-74, -23, 46, -60, 32, 68, -41, -35, 
        -82, -29, 11, 5, -84, -5, -39, 63, 
        38, -40, -14, 25, -16, -33, -78, 77, 
        38, 70, 39, -59, 60, -50, -36, 64, 
        39, -20, -48, 46, 3, -71, 48, -66, 
        31, 10, 42, -17, 68, 23, 4, -15, 
        21, 54, 32, 28, -42, -12, 23, -25, 
        9, -57, 33, 34, 42, -46, 30, 24},

        {-35, -12, 32, 0, -3, 18, 30, -28, 
        7, -31, 46, -17, 39, 3, -4, -30, 
        -13, 22, 11, 43, 36, 42, -32, 17, 
        34, 7, 37, 27, 33, -18, 25, 27, 
        9, 22, 18, -29, 10, 3, 37, -2, 
        -23, -34, -24, -11, 17, 26, -9, -9, 
        -17, -9, -38, 0, -20, -25, -20, -20, 
        -5, 4, -11, 14, 20, 6, -19, -26},

        {-9, -32, 16, 24, -18, -29, -24, 22, 
        7, 11, -24, -31, 9, -33, 14, 10, 
        -19, -2, -25, 7, 25, -8, 16, 6, 
        7, -15, 12, -12, 15, -9, 6, -2, 
        14, 23, 17, -2, 10, 22, -8, 11, 
        1, 10, 9, 7, 5, 26, -23, -7, 
        0, 3, 0, -8, 11, -30, 23, -1, 
        -31, 9, -25, -15, -15, -24, 0, 6},

        {16, 18, 7, -8, 23, 16, -6, 12, 
        -10, 19, 16, 23, -7, 26, 21, 14, 
        -7, 34, -14, 0, 1, -2, 21, 10, 
        1, 20, 38, 44, 51, 2, 18, 10, 
        5, -15, 8, 12, 18, 32, 7, 1, 
        -24, 8, 3, 27, -7, 14, -2, -6, 
        -1, 17, -6, -5, 0, 16, -3, -7, 
        2, -35, -1, -19, -12, -25, -7, 1},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        18, 7, 3, 0, 9, -3, 1, 0, 
        15, 13, 9, 23, 12, 7, -3, 13, 
        -19, -14, -10, 3, -15, -22, -22, -37, 
        -21, -21, -20, -8, 0, -26, -20, -32, 
        -25, -17, -34, -18, -12, -26, 12, -28, 
        -32, -23, -34, -37, -16, -3, 24, -38, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        double eps[6][64] = 
        {{19, 5, -31, -15, 20, 23, 28, 6, 
        -31, -7, -17, -18, -35, 20, 11, -10, 
        27, 27, 23, -11, -6, 30, 11, 19, 
        -31, -13, 7, -8, -19, -7, 18, -5, 
        23, 15, -15, 1, 10, 2, 12, -19, 
        17, -7, -4, -1, 15, 0, 1, -28, 
        4, 0, 6, -26, 0, 0, -3, -21, 
        -28, -5, -29, -40, -15, -7, -36, -3},

        {-74, -65, 56, -38, 9, -50, 19, -34, 
        -39, 2, -72, 35, 89, 75, 30, -16, 
        -23, 65, 70, 4, 1, 58, 8, -23, 
        9, -42, -8, 1, 64, -73, 58, -40, 
        -65, -62, 23, -25, -18, 48, -81, -51, 
        66, 68, 5, -16, -71, 71, 59, 25, 
        -13, -50, 8, 15, 38, 72, 37, 43, 
        10, 12, -59, -76, -14, 33, -27, -13},

        {-13, 22, 33, -28, 7, 30, -6, -26, 
        3, 45, 33, -24, 16, 25, 16, 38, 
        0, 31, 18, 17, 11, -2, -22, 7, 
        5, -8, 11, -4, 24, 20, 24, 12, 
        9, -15, -22, -13, 11, -13, -29, 14, 
        16, -32, -9, 6, -30, -44, -12, 5, 
        6, 27, 21, -33, 27, -37, 32, -18, 
        -14, -20, 22, 0, -13, 2, 19, -13},

        {13, -25, 22, 17, 10, 22, 23, -27, 
        9, 9, 0, 23, 10, 0, -24, -16, 
        0, -19, -10, 21, -6, -21, -24, -1, 
        0, 29, -25, 34, 19, -12, -20, 23, 
        0, 1, 18, -4, -4, -8, 0, -19, 
        -25, -2, 12, 25, 1, -7, 22, 15, 
        2, -15, -11, -27, -3, 7, 11, 11, 
        -21, -1, -43, -23, 1, -30, 17, -12},

        {15, 2, -15, 0, 1, 7, 19, -2, 
        18, -15, 20, 24, 23, 7, -1, 7, 
        -16, -20, -13, 29, -3, 24, 21, 22, 
        -8, 3, 0, 15, 3, 4, 18, 10, 
        0, 2, 12, 7, 22, 15, 9, -10, 
        9, -9, -2, -13, 23, 15, -6, -20, 
        -19, -19, -4, 13, 6, -3, -21, -4, 
        1, -27, -7, -17, -25, -13, -26, 3},

        {0, 0, 0, 0, 0, 0, 0, 0, 
        40, 27, 29, 20, 9, 9, 12, 21, 
        46, 37, 26, 28, 18, 15, 19, 38, 
        35, 22, 15, 0, -3, 7, 20, 10, 
        11, 10, -5, -15, -18, -14, 3, -8, 
        10, 7, -10, -10, -9, -1, -11, -15, 
        4, 10, 0, -1, -15, -6, -10, -14, 
        0, 0, 0, 0, 0, 0, 0, 0}};

        double material[6] = {0, 929, 476, 318, 284, 117};
        
        Guesser();

        double evaluate(); //returns static eval
        double evaluateScratch(); //computes all psqts and mobility from scratch, then evaluates

        void outputs(std::string);
};

Guesser::Guesser(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            mps[i][j] += material[i];
            eps[i][j] += material[i];
        }
    }
}

double Guesser::evaluate(){
    //return ((scores[toMove] - scores[!toMove]) * gamePhase + (eScores[toMove] - eScores[!toMove]) * (totalPhase - gamePhase)) / totalPhase;
    //return scores[toMove] - scores[!toMove];

    double m = (scores[toMove] - scores[!toMove]) * gamePhase;
    double e = (eScores[toMove] - eScores[!toMove]) * (totalPhase - gamePhase);

    return (m + e) / totalPhase;
}

double Guesser::evaluateScratch(){
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

    return evaluate();
}

void Guesser::outputs(std::string fn){
    std::ofstream pst(fn);

    pst << "int mps[6][64] = \n{{";
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 63; j++){
            pst << (int) (mps[i][j] - material[i]) << ", ";
            if (j % 8 == 7){
                pst << '\n';
            }
        }
        pst << (int) (mps[i][63] - material[i]) << "},\n\n{";
    }

    pst << "\n\nint eps[6][64] = \n{{";
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 63; j++){
            pst << (int) (eps[i][j] - material[i]) << ", ";
            if (j % 8 == 7){
                pst << '\n';
            }
        }
        pst << (int) (eps[i][63] - material[i]) << "},\n\n{";
    }

    pst << "\nint material[6] = {";
    for (int i = 0; i < 6; i++){
        pst << (int) material[i] << ", ";
    }
}

class Tuna{
    public:

    Guesser e;

    std::ifstream inData;
    uint32_t numPos;

    double* midlse; //array for least-squared error
    double* endlse;
    double* matlse;

    double sigk;
    double learnRate;

    Tuna();
    ~Tuna();

    //int pdonsq(int, int);
    int pdonbd(int);

    double sigmoid(double);
    double sigmoidDiff(double);
    void optimizeK(double, double); //optimize between start and end

    double loadLine(std::string);
    void openDataFile(std::string);

    double errorSum();

    void canTune(Bitboard&, Bitboard&, int);

    //double miDeriv(double, int); //middlegame psqt derivative
    //double eiDeriv(double, int); //endgame psqt derivative
    //double vkDeriv(double, int); //base material value derivative

    void updates(); // update the terms.

    void updateOnPosition(std::string);

    void zeroLists();
    void tune(int);

    void changeWeights();
};

Tuna::Tuna(){
    midlse = new double[768];
    endlse = new double[768];
    matlse = new double[5];

    zeroLists();

    //sigk = log(10) / 400;
    //optimizeK(0.0, 0.01);
}

Tuna::~Tuna(){
    delete[] midlse;
    delete[] endlse;
    delete[] matlse;

    inData.close();
}

int Tuna::pdonbd(int pt){ //piece difference on board
    int ewp = __builtin_popcountll(e.sides[1] & e.pieces[pt]);
    int ebp = __builtin_popcountll(e.sides[0] & e.pieces[pt]);

    return ewp - ebp;
}

double Tuna::sigmoid(double x){
    return 1.0 / (1.0 + exp(-sigk * x));
}

double Tuna::sigmoidDiff(double x){
    double s = sigmoid(x);
    return sigk * s * (1 - s);
}

void Tuna::optimizeK(double a, double b){
    int steps = 5; //number of steps per precision
    int grains = 5; //layers of precision

    std::cout << "Optimizing K...\n";

    std::cout << "Steps: " << steps << '\n';
    std::cout << "Grains: " << grains << '\n';

    double best, val, min = 1.0; //best k, trial error, minimum error
    double stride; //stride

    for (int i = 0; i < grains; i++){
        stride = (b - a) / steps; //interval over num steps
        for (int j = 0; j <= steps; j++){ //for each interval endpoint
            sigk = a + j * stride; // create new value of k
            val = errorSum(); //try it
            if (val < min){ //if its good, save the value. 
                best = sigk;
                min = val;
            }
        }

        a = best - stride;
        b = best + stride;

        std::cout << i + 1 << " of " << grains << " intervals completed.\n";
    }

    sigk = best;
}

double Tuna::loadLine(std::string line){
    std::stringstream ls(line);
    std::string fen, r;

    std::getline(ls, fen, '|');
    std::getline(ls, r, '|');
    std::getline(ls, r, '|');

    e.readFen(fen);
    //e.print();

    return stod(r);
}

void Tuna::openDataFile(std::string fname){
    inData.open(fname);
    numPos = 0;
    std::string burn;
    while (std::getline(inData, burn)){
        numPos++;
    }

    inData.clear();
    inData.seekg(0); //set back to beginning
}

double Tuna::errorSum(){
    inData.clear();
    inData.seekg(0); //set back to beginning

    double errSum = 0.0;
    double qi, res;
    std::string burn;

    while (std::getline(inData, burn)){
        res = loadLine(burn);
        qi = e.evaluateScratch() * (2 * e.who() - 1);

        errSum += pow(res - sigmoid(qi), 2.0);
    }

    inData.clear();
    inData.seekg(0); //set back to beginning

    return errSum / numPos; //mean squarred error
}

void Tuna::updates(){
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 64; j++){
            e.mps[i][j] -= learnRate * midlse[(i << 6) + j];
            e.eps[i][j] -= learnRate * endlse[(i << 6) + j];
        }
    }

    for (int i = 0; i < 5; i++){
        e.material[i + 1] -= learnRate * matlse[i];
        for (int j = 0; j < 64; j++){
            e.mps[i + 1][j] -= learnRate * matlse[i];
            e.eps[i + 1][j] -= learnRate * matlse[i];
        }
    }
}

void Tuna::canTune(Bitboard& w, Bitboard& b, int pt){
    w = e.sides[1] & e.pieces[pt];
    b = e.sides[0] & e.pieces[pt];

    Bitboard col = __builtin_bswap64(b) & w;
    col |= __builtin_bswap64(col);

    w &= ~col;
    b &= ~col;
}

void Tuna::updateOnPosition(std::string pos){
    double ri = loadLine(pos);

    double s = e.evaluateScratch() * (2 * e.who() - 1); // white-centered score

    double factor = 2.0 * (ri - sigmoid(s)) * (-sigmoidDiff(s)) / numPos;

    for (int i = 1; i < 6; i++){
        matlse[i - 1] += pdonbd(i) * factor;
    }

    int f;
    Bitboard wtun, btun;
    for (int i = 0; i < 6; i++){
        canTune(wtun, btun, i);

        while (wtun){
            f = __builtin_ctzll(wtun);

            midlse[(i << 6) + f] += factor * e.gamePhase / 128.0;
            endlse[(i << 6) + f] += factor * (128.0 - e.gamePhase) / 128.0;

            wtun ^= (1ULL << f);
        }

        while (btun){
            f = __builtin_ctzll(btun);

            midlse[(i << 6) + (f ^ 56)] -= factor * e.gamePhase / 128.0;
            endlse[(i << 6) + (f ^ 56)] -= factor * (128.0 - e.gamePhase) / 128.0;
            
            btun ^= (1ULL << f);
        }
    }
}

void Tuna::zeroLists(){
    for (int i = 0; i < 768; i++){
        midlse[i] = 0.0;
        endlse[i] = 0.0;
    }
    
    for (int i = 0; i < 5; i++){
        matlse[i] = 0.0;
    }
}

void Tuna::tune(int epochs){
    std::string burn;
    for (int i = 0; i < epochs; i++){
        zeroLists();
        inData.clear();
        inData.seekg(0);

        //std::cout << "here\n";

        while (std::getline(inData, burn)){
            //std::cout << burn << '\n';
            updateOnPosition(burn);
        }

        updates();

        std::cout << i + 1 << " of " << epochs << " epochs finished\n";
        
        if (i % 5 == 0){
            std::cout << "MSE = " << errorSum() << '\n';
            e.outputs("testpst.txt");
        }

    }
}

int main(int argc, char* argv[]){
    if (argc != 4){
        std::cout << "Correct Usage: ./wtuna fileName learnRate epochs\n";
        return 0;
    }

    //srand(time(0));
    //Bitboards::initSlideAttacks();

    Tuna t;

    t.openDataFile(argv[1]);
    
    t.optimizeK(0.0, 0.01);
    std::cout << "Optimal K: " << t.sigk << "\nMSE: " << t.errorSum() << '\n';

    t.learnRate = std::stod(std::string(argv[2]));
    std::cout << "Learning Rate: " << t.learnRate << '\n';

    int epo = stoi(std::string(argv[3]));
    std::cout << "Epochs: " << epo << '\n';

    t.tune(epo);

    //std::cout << "MSE: " << t.errorSum() << '\n';

    t.e.outputs("testpst.txt");

    return 0;
}











