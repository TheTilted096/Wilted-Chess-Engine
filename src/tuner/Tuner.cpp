#include "Guesser.h"

class Tuner{
    public:
        Guesser g;
        float k = 0.0058; //ln(10) / 400, as originally prescribed

        //float gamma = 0.9;

        Table<float, 6, 64> midacc;
        Table<float, 6, 64> endacc;
        std::array<float, 6> matacc;
        std::array<float, 6> endmatacc;
        float tempoacc;
        
        float sig(const float&);
        float sigDiff(const float&);

        void clear();

        void updateOnPosition(const Bullet&);
        void step(const std::size_t&, const float&);

};

float Tuner::sig(const float& x){
    return 1.0 / (1.0 + exp(-k * x));
}

float Tuner::sigDiff(const float& x){
    float s = sig(x);
    return k * s * (1 - s);
}

void Tuner::clear(){
    std::fill(&midacc[0][0], &midacc[0][0] + 384, 0.0f);
    std::fill(&endacc[0][0], &endacc[0][0] + 384, 0.0f);
    matacc.fill(0.0);
    endmatacc.fill(0.0);
    tempoacc = 0.0;
}

void Tuner::updateOnPosition(const Bullet& b){
    float p = g.fromBullet(b);
    float factor = 2.0 * (sig(p) - (b.result / 2.0)) * sigDiff(p);
    int n, m;

    float gp = g.gamePhase / 128.0;

    for (Piece i = Queen; i < None; i++){
        matacc[i] += factor * g.pieceDiff[i] * gp;
        endmatacc[i] += factor * g.pieceDiff[i] * (1.0 - gp);
    }

    for (int i = 0; i < g.numStm; i++){
        n = g.stmactive[i] >> 6;
        m = g.stmactive[i] & 63U;
        midacc[n][m] += factor * gp;
        endacc[n][m] += factor * (1.0 - gp);
    }

    for (int i = 0; i < g.numNstm; i++){
        n = g.nstmactive[i] >> 6;
        m = g.nstmactive[i] & 63U;
        midacc[n][m] -= factor * gp;
        endacc[n][m] -= factor * (1.0 - gp);
    }

    tempoacc += factor;
}

void Tuner::step(const std::size_t& N, const float& n){
    for (int i = 0; i < 6; i++){
        g.material[i] -= (n * matacc[i] / N);
        g.endmaterial[i] -= (n * endmatacc[i] / N);

        for (int j = 0; j < 64; j++){
            g.midpst[i][j] -= (n * midacc[i][j] / N);
            g.endpst[i][j] -= (n * endacc[i][j] / N);
        }
    }

    g.tempo -= (n * tempoacc / N);
}

int main(int argc, char* argv[]){   
    if (argc != 4){
        std::cout << "Correct Use: ./WiltedTuna-1-1-0 datafile epochs learnRate";
        return 0;
    }

    std::ifstream datain(argv[1], std::ios::binary);

    const std::size_t numData = [&](){
        datain.seekg(0, std::ios::end);

        std::size_t r = datain.tellg() / sizeof(Bullet);

        datain.seekg(0);

        return r;
    }();

    Hash seed = std::chrono::steady_clock::now().time_since_epoch().count();

    constexpr std::size_t BATCH_SIZE = 16834;

    float eta = std::stof(std::string(argv[3]));

    std::size_t idx;
    Bullet b;

    int epochs = std::stoi(std::string(argv[2]));

    Tuner t;

    std::cout << "Beginning SGD...\n";
    std::cout << "Batch Size: " << BATCH_SIZE << '\n';
    std::cout << "Learning Rate: " << eta << '\n';
    std::cout << "Dataset Size: " << numData << '\n';

    //float bestesum = 1.0;

    for (int i = 0; i < epochs; i++){
        if ((i % 100) == 0){
            float esum = 0.0;

            for (std::size_t jj = 0; jj < numData; jj++){
                datain.seekg(sizeof(Bullet) * jj);
                datain.read(reinterpret_cast<char*>(&b), sizeof(Bullet));
                float p = t.g.fromBullet(b);

                esum += pow(t.sig(p) - b.result / 2.0, 2.0);
            }

            esum /= numData;

            std::cout << "Error Before Epoch " << i << ": " << esum << '\n';

            /*
            if (esum > bestesum){
                std::cout << i << " epochs run.\n";
                break;
            }

            bestesum = esum;
            */

            if ((i % 100) == 0){
                t.g.report(i); 
            }
        }


        t.clear();

        for (std::size_t j = 0; j < BATCH_SIZE; j++){
            idx = randomize(seed) % numData;

            datain.seekg(sizeof(Bullet) * idx);
            datain.read(reinterpret_cast<char*>(&b), sizeof(Bullet));

            t.updateOnPosition(b);
        }

        t.step(BATCH_SIZE, eta);    

        if ((i % 25) == 0){
            std::cout << "Completed Epoch " << i << '\n';
        }
    }

    datain.close();

    return 0;
}