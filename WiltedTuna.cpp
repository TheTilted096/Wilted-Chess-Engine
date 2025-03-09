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

        double material[6] = {0, 900, 500, 350, 300, 100};
        static constexpr double phases[6] = {0, 14, 8, 5, 4, 2};
        static constexpr double totalPhase = 128;

        double mps[6][64] = 
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

        double eps[6][64] =
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

    pst << "double mps[6][64] = \n{{";
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 63; j++){
            pst << (int) (mps[i][j] - material[i]) << ", ";
            if (j % 8 == 7){
                pst << '\n';
            }
        }
        pst << (int) (mps[i][63] - material[i]) << "},\n\n{";
    }

    pst << "\n\n\ndouble eps[6][64] = \n{{";
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 63; j++){
            pst << (int) (eps[i][j] - material[i]) << ", ";
            if (j % 8 == 7){
                pst << '\n';
            }
        }
        pst << (int) (eps[i][63] - material[i]) << "},\n\n{";
    }

    pst << "\n\ndouble material[6] = {";
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

        //if (i % 50 == 0){ 
            std::cout << i + 1 << " of " << epochs << " epochs finished. MSE = " << errorSum() << '\n';
        //}
    }
}

int main(int argc, char* argv[]){
    if (argc != 2){
        std::cout << "Correct Usage: ./wtuna fileName\n";
        return 0;
    }

    //srand(time(0));
    //Bitboards::initSlideAttacks();

    Tuna t;

    t.openDataFile(argv[1]);
    
    //t.optimizeK(0.0, 0.01);
    t.sigk = log(10) / 400;
    std::cout << "Optimal K: " << t.sigk << "\nMSE: " << t.errorSum() << '\n';

    t.learnRate = 100000;
    std::cout << "Learning Rate: " << t.learnRate << '\n';

    t.tune(100);

    std::cout << "MSE: " << t.errorSum() << '\n';

    t.e.outputs("testpst.txt");

    return 0;
}











