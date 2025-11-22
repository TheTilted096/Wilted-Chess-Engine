// Function Implementations for Evaluation

#include "Evaluator.h"

#ifndef EVALFILE
#define EVALFILE "wilted-net-1-0.bin"
#endif

INCBIN(WiltedNet, EVALFILE);

Table<int16_t, 2, 6, 64, Network::L1_SIZE> Network::inputWeights;
std::array<int16_t, Network::L1_SIZE> Network::inputBiases;

Table<int16_t, 2, Network::L1_SIZE> Network::outputWeights;
int16_t Network::outputBias;

template<typename Reader>
static void loadWeightsFromReader(Reader& reader){
    int16_t w = 0;

    for (int i = 0; i < 2; i++){
        for (int j = 0; j < 6; j++){
            for (int k = 0; k < 64; k++){
                for (int l = 0; l < Network::L1_SIZE; l++){
                    reader(&w, 2);
                    Network::inputWeights[!i][5 - j][56 ^ k][l] = w;
                }
            }
        }
    }

    reader(&Network::inputBiases, 2 * Network::L1_SIZE);
    reader(&Network::outputWeights[1], 2 * Network::L1_SIZE);
    reader(&Network::outputWeights[0], 2 * Network::L1_SIZE);
    reader(&Network::outputBias, 2);
}

void Network::loadnet(const std::string& filename){
    if (filename.empty()){
        // Load from embedded binary data
        const unsigned char* data = gWiltedNetData;
        std::size_t offset = 0;

        auto memoryReader = [&data, &offset](void* dest, std::size_t size){
            std::memcpy(dest, data + offset, size);
            offset += size;
        };

        loadWeightsFromReader(memoryReader);
    } else {
        // Load from user-specified file
        std::ifstream in(filename, std::ios::binary);

        auto fileReader = [&in](void* dest, std::size_t size){
            in.read(reinterpret_cast<char*>(dest), size);
        };

        loadWeightsFromReader(fileReader);
    }
}

Evaluator::Evaluator(){}

Score Evaluator::inference() const{
    const Network::Accumulator& acc = accStack[accIdx];

    int eval = 0;
    const Color& stm = pos->toMove;

    for (int i = 0; i < Network::L1_SIZE; i++){
        int16_t stmTerm = Network::crelu(acc[stm][i]);
        int16_t nstmTerm = Network::crelu(acc[!stm][i]);

        eval += static_cast<int16_t>(stmTerm * Network::outputWeights[1][i]) * stmTerm;
        eval += static_cast<int16_t>(nstmTerm * Network::outputWeights[0][i]) * nstmTerm;
    }

    eval /= Network::QA;
    eval += Network::outputBias;
    eval *= Network::SCALE;
    eval /= (Network::QA * Network::QB);

    return eval;
}

Score Evaluator::refresh(){
    accIdx = 0;
    Network::Accumulator& acc = accStack[accIdx];

    acc[0] = Network::inputBiases;
    acc[1] = Network::inputBiases;

    Bitboard pcs;
    Square p;

    for (Piece i = King; i < None; i++){
        pcs = pos->those(White, i);
        while (pcs){
            p = popLeastBit(pcs);
            add(White, i, p);
        }

        pcs = pos->those(Black, i);
        while (pcs){
            p = popLeastBit(pcs);
            add(Black, i, p);
        }
    }

    /*
    for (int i = 0; i < Network::L1_SIZE; i++){
        std::cout << acc[0][i] << ' ';
    }
    std::cout << '\n';
    for (int i = 0; i < Network::L1_SIZE; i++){
        std::cout << acc[1][i] << ' ';
    }
    */

    return inference();
}

void Evaluator::add(const Color& c, const Piece& p, const Square& s){
    Network::Accumulator& acc = accStack[accIdx];
    for (int i = 0; i < Network::L1_SIZE; i++){
        acc[c][i] += Network::inputWeights[1][p][s ^ (56 * !c)][i];
        acc[!c][i] += Network::inputWeights[0][p][s ^ (56 * c)][i];
    }
}

void Evaluator::sub(const Color& c, const Piece& p, const Square& s){
    Network::Accumulator& acc = accStack[accIdx];
    for (int i = 0; i < Network::L1_SIZE; i++){
        acc[c][i] -= Network::inputWeights[1][p][s ^ (56 * !c)][i];
        acc[!c][i] -= Network::inputWeights[0][p][s ^ (56 * c)][i];
    }
}

void Evaluator::doMove(const Move& m){
    accIdx++;

    accStack[accIdx] = accStack[accIdx - 1]; //copy in previous values

    Square start = m.from();
    Square end = m.to();
    Piece typei = m.moving();
    Piece typef = m.ending();

    Piece victim = m.captured();

    Color us = flip(pos->toMove); //side that made the move, could be dangerous when attempting lazy updates

    if (victim){
        bool passant = m.epCapture();

        Square target = static_cast<Square>(end + passant * ((us << 4) - 8));
        sub(flip(us), victim, target);
    }

    add(us, typef, end);
    sub(us, typei, start);

    if (m.castling()){
        if (m.kingCastle()){ //kingside castle 
            start = pos->kingRookFrom[us];
            end = pos->kingRookTo[us];
        } else {
            start = pos->queenRookFrom[us];
            end = pos->queenRookTo[us];
        }

        add(us, Rook, end);
        sub(us, Rook, start);
    }
}

