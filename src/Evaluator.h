// Class Definition for Evaluation

#include "Position.h"

#define INCBIN_PREFIX g
#include "../third_party/incbin.h"

INCBIN_EXTERN(WiltedNet);

class Network{
    public:
        static constexpr std::size_t L1_SIZE = 256;
        static constexpr int16_t QA = 255;
        static constexpr int16_t QB = 64;
        static constexpr int16_t SCALE = 200;

        // stm = 1
        // pawn = 5, king = 0
        // a8 = 0

        static Table<int16_t, 2, 6, 64, L1_SIZE> inputWeights;
        static std::array<int16_t, L1_SIZE> inputBiases;

        static Table<int16_t, 2, L1_SIZE> outputWeights;
        static int16_t outputBias;

        static constexpr int16_t crelu(const int16_t& x){
            return std::clamp(x, static_cast<int16_t>(0), QA);
        }

        static void loadnet(const std::string& filename = "");

        using Accumulator = Table<int16_t, 2, L1_SIZE>;
};

class Evaluator{
    public:
        Position* pos;
        
        std::array<Network::Accumulator, MAX_PLY + 32> accStack;
        Index accIdx = 0;

        Evaluator();

        void assign(Position* pp){ pos = pp; }

        Score inference() const;
        Score refresh();

        void add(const Color&, const Piece&, const Square&);
        void sub(const Color&, const Piece&, const Square&);

        void doMove(const Move& m); //no lazy updates
        void undoMove(){ accIdx--; }
};