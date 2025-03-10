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











