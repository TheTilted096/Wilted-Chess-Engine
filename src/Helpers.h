// Helper Functions

#pragma once

#include "Types.h"

template <typename T, std::size_t J, std::size_t... K> 
struct MultiArrayHelper{
    using Inside = typename MultiArrayHelper<T, K...>::type;
    using type = std::array<Inside, J>;
};

template <typename T, std::size_t J>
struct MultiArrayHelper<T, J>{
    using type = std::array<T, J>;
};

template <typename T, std::size_t J, std::size_t... K> using Table = typename MultiArrayHelper<T, J, K...>::type;
//"Table" might be changed later

constexpr Hash randomize(Hash &s){
    Hash x = s;
    
    x ^= (x >> 12);
    x ^= (x << 25);
    x ^= (x >> 27);

    s = x;

    return x * 0x2545F4914F6CDD1DULL;
}

inline void printAsBitboard(const Bitboard& bb){
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            std::cout << ((bb >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

constexpr Bitboard squareBitboard(const Square& s){
    return 1ULL << s;
}

constexpr Square getLeastBit(const Bitboard& b){
    return static_cast<Square>(std::countr_zero<Bitboard>(b));
}

constexpr Square popLeastBit(Bitboard& b){
    Square s = getLeastBit(b);
    b &= b - 1;
    return s;

    /* alternatively,
    Square s = static_cast<Square>(std::popcount<Bitboard>((b & -b) - 1)));
    
    and

    b ^= (1ULL << s);
    */
}

constexpr Bitboard manual_pdep_u64(const Bitboard& src, const Bitboard& mask){
    Bitboard m = mask;
    Bitboard result = 0ULL;
    Index i = 0; //bit position of source
    Square f; //position of mask lsb
    while (m){
        f = popLeastBit(m);
        result |= (((src >> i) & 1ULL) << f);
        i++;
    }
    return result;
}



