// Zobrist Keys

#include "Helpers.h"

namespace Zobrist{

constexpr Table<Hash, 2, 6, 64> pieceKeys = [](){
    Table<Hash, 2, 6, 64> kys{};
    
    Hash pieceSeed = 0x1BB5C377A8DEA1CAULL;

    for (Index i = 0; i < 6; i++){
        for (Index j = 0; j < 64; j++){
            kys[Black][i][j] = randomize(pieceSeed);
            kys[White][i][j] = randomize(pieceSeed);
        }
    }

    return kys;
}();

constexpr std::array<Hash, 16> castleKeys = [](){
    std::array<Hash, 16> kys{};

    Hash castleSeed = 0x62B4826C492B9A00ULL;

    for (Index i = 0; i < 16; i++){
        kys[i] = randomize(castleSeed);
    }

    return kys;
}();

constexpr std::array<Hash, 8> passantKeys = [](){
    std::array<Hash, 8> kys{};

    Hash passantSeed = 0x53E76BEBA30448CDULL;

    for (Index i = 0; i < 8; i++){
        kys[i] = randomize(passantSeed);
    }

    return kys;
}();

constexpr Hash turnKey = 0x999E26B3CAB49F17ULL;

}