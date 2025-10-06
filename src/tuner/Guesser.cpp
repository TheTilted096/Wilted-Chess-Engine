#include "Guesser.h"

float Guesser::fromBullet(const Bullet& b){
    numStm = 0;
    numNstm = 0;

    float m = 0.0, e = 0.0;
    gamePhase = 0.0;

    Bitboard occ = b.occ;

    Square f;
    Piece p;
    Color c;
    uint8_t nibble;
    int ind;

    pieceDiff.fill(0);

    for (int x = 0; occ; x++){
        nibble = (b.pcs[x >> 1] >> ((x & 1) << 2)) & 0xF;

        c = static_cast<Color>(!(nibble >> 3));

        f = static_cast<Square>(popLeastBit(occ) ^ (56 * c));

        p = static_cast<Piece>(5 - (nibble & 7));

        ind = (p << 6) + f;

        gamePhase += phases[p];

        if (c){
            stmactive[numStm] = ind;
            numStm++;

            m += midpst[p][f];
            e += endpst[p][f];
            pieceDiff[p]++;
        } else {
            nstmactive[numNstm] = ind;
            numNstm++;

            m -= midpst[p][f];
            e -= endpst[p][f];
            pieceDiff[p]--;
        }
    }

    float s = (m * gamePhase + e * (totalPhase - gamePhase)) / totalPhase;
    for (int i = 1; i < 6; i++){
        s += material[i] * pieceDiff[i] * gamePhase / totalPhase;
        s += endmaterial[i] * pieceDiff[i] * (totalPhase - gamePhase) / totalPhase;
    }

    s += tempo;

    return s;
}

void Guesser::report(int iter){
    std::ofstream output("output" + std::to_string(iter) + ".txt");

    output << "Table<Score, 6, 64> midpst = {{\n";
    for (int i = 0; i < 6; i++){
        output << '{';
        for (int j = 0; j < 64; j++){
            output << (int)midpst[i][j] << ", ";
            if ((j & 7) == 7 and (j != 63)){
                output << '\n';
            }
        }
        output << "}, \n\n";
    }

    output << "Table<Score, 6, 64> endpst = {{\n";
    for (int i = 0; i < 6; i++){
        output << '{';
        for (int j = 0; j < 64; j++){
            output << (int)endpst[i][j] << ", ";
            if ((j & 7) == 7 and (j != 63)){
                output << '\n';
            }
        }
        output << "}, \n\n";
    }

    output << "std::array<Score, 6> material = {";
    for (int i = 0; i < 5; i++){
        output << (int)material[i] << ", ";
    }
    output << (int)material[5] << "};\n";

    output << "std::array<Score, 6> endmaterial = {";
    for (int i = 0; i < 5; i++){
        output << (int)endmaterial[i] << ", ";
    }
    output << (int)endmaterial[5] << "};\n";

    output << "Score tempo = " << (int)tempo << ";\n";

}