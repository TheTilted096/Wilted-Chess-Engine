/*
Very Base Class for the Wilted Chess Engine
Class Definition of Bitboards

TheTilted096 12-18-24

Contains board representation and elementary makeMove

*/

//#include <cassert>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

typedef uint64_t Bitboard;
typedef uint64_t Hash;
//typedef unsigned int Move;

class Bitboards{
    public:
        //Pawn Lookup Table
        
        //King and Knight Lookup Table
        static constexpr Bitboard KingAttacks[64] = 
        {0x302ULL, 0x705ULL, 0xE0AULL, 0x1C14ULL, 0x3828ULL, 0x7050ULL, 0xE0A0ULL, 0xC040ULL, 
        0x30203ULL, 0x70507ULL, 0xE0A0EULL, 0x1C141CULL, 0x382838ULL, 0x705070ULL, 0xE0A0E0ULL, 0xC040C0ULL, 
        0x3020300ULL, 0x7050700ULL, 0xE0A0E00ULL, 0x1C141C00ULL, 0x38283800ULL, 0x70507000ULL, 0xE0A0E000ULL, 0xC040C000ULL, 
        0x302030000ULL, 0x705070000ULL, 0xE0A0E0000ULL, 0x1C141C0000ULL, 0x3828380000ULL, 0x7050700000ULL, 0xE0A0E00000ULL, 0xC040C00000ULL, 
        0x30203000000ULL, 0x70507000000ULL, 0xE0A0E000000ULL, 0x1C141C000000ULL, 0x382838000000ULL, 0x705070000000ULL, 0xE0A0E0000000ULL, 0xC040C0000000ULL, 
        0x3020300000000ULL, 0x7050700000000ULL, 0xE0A0E00000000ULL, 0x1C141C00000000ULL, 0x38283800000000ULL, 0x70507000000000ULL, 0xE0A0E000000000ULL, 0xC040C000000000ULL, 
        0x302030000000000ULL, 0x705070000000000ULL, 0xE0A0E0000000000ULL, 0x1C141C0000000000ULL, 0x3828380000000000ULL, 0x7050700000000000ULL, 0xE0A0E00000000000ULL, 0xC040C00000000000ULL, 
        0x203000000000000ULL, 0x507000000000000ULL, 0xA0E000000000000ULL, 0x141C000000000000ULL, 0x2838000000000000ULL, 0x5070000000000000ULL, 0xA0E0000000000000ULL, 0x40C0000000000000ULL};

        static constexpr Bitboard KnightAttacks[64] = 
        {0x20400ULL, 0x50800ULL, 0xA1100ULL, 0x142200ULL, 0x284400ULL, 0x508800ULL, 0xA01000ULL, 0x402000ULL, 
        0x2040004ULL, 0x5080008ULL, 0xA110011ULL, 0x14220022ULL, 0x28440044ULL, 0x50880088ULL, 0xA0100010ULL, 0x40200020ULL, 
        0x204000402ULL, 0x508000805ULL, 0xA1100110AULL, 0x1422002214ULL, 0x2844004428ULL, 0x5088008850ULL, 0xA0100010A0ULL, 0x4020002040ULL, 
        0x20400040200ULL, 0x50800080500ULL, 0xA1100110A00ULL, 0x142200221400ULL, 0x284400442800ULL, 0x508800885000ULL, 0xA0100010A000ULL, 0x402000204000ULL, 
        0x2040004020000ULL, 0x5080008050000ULL, 0xA1100110A0000ULL, 0x14220022140000ULL, 0x28440044280000ULL, 0x50880088500000ULL, 0xA0100010A00000ULL, 0x40200020400000ULL, 
        0x204000402000000ULL, 0x508000805000000ULL, 0xA1100110A000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL, 0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL, 
        0x400040200000000ULL, 0x800080500000000ULL, 0x1100110A00000000ULL, 0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010A000000000ULL, 0x2000204000000000ULL, 
        0x4020000000000ULL, 0x8050000000000ULL, 0x110A0000000000ULL, 0x22140000000000ULL, 0x44280000000000ULL, 0x88500000000000ULL, 0x10A00000000000ULL, 0x20400000000000ULL};
        
        static constexpr Bitboard plt[2][64] =
        {{0x200ULL, 0x500ULL, 0xA00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xA000ULL, 0x4000ULL, 
        0x20000ULL, 0x50000ULL, 0xA0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xA00000ULL, 0x400000ULL, 
        0x2000000ULL, 0x5000000ULL, 0xA000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xA0000000ULL, 0x40000000ULL, 
        0x200000000ULL, 0x500000000ULL, 0xA00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xA000000000ULL, 0x4000000000ULL, 
        0x20000000000ULL, 0x50000000000ULL, 0xA0000000000ULL, 0x140000000000ULL, 0x280000000000ULL, 0x500000000000ULL, 0xA00000000000ULL, 0x400000000000ULL, 
        0x2000000000000ULL, 0x5000000000000ULL, 0xA000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xA0000000000000ULL, 0x40000000000000ULL, 
        0x200000000000000ULL, 0x500000000000000ULL, 0xA00000000000000ULL, 0x1400000000000000ULL, 0x2800000000000000ULL, 0x5000000000000000ULL, 0xA000000000000000ULL, 0x4000000000000000ULL, 
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL}, {
        0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 
        0x2ULL, 0x5ULL, 0xAULL, 0x14ULL, 0x28ULL, 0x50ULL, 0xA0ULL, 0x40ULL, 
        0x200ULL, 0x500ULL, 0xA00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xA000ULL, 0x4000ULL, 
        0x20000ULL, 0x50000ULL, 0xA0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xA00000ULL, 0x400000ULL, 
        0x2000000ULL, 0x5000000ULL, 0xA000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xA0000000ULL, 0x40000000ULL, 
        0x200000000ULL, 0x500000000ULL, 0xA00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xA000000000ULL, 0x4000000000ULL, 
        0x20000000000ULL, 0x50000000000ULL, 0xA0000000000ULL, 0x140000000000ULL, 0x280000000000ULL, 0x500000000000ULL, 0xA00000000000ULL, 0x400000000000ULL, 
        0x2000000000000ULL, 0x5000000000000ULL, 0xA000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xA0000000000000ULL, 0x40000000000000ULL}};

        //Horizontal Lookup Table
        static constexpr uint8_t hlt[8][64] =
        {{0xFE, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x3E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x7E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x3E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2, 
        0x1E, 0x2, 0x6, 0x2, 0xE, 0x2, 0x6, 0x2}, {
        0x0, 0xFD, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x3D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x7D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x3D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5, 
        0x0, 0x1D, 0x0, 0x5, 0x0, 0xD, 0x0, 0x5}, {
        0x0, 0x0, 0xFB, 0xFA, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x3B, 0x3A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x7B, 0x7A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x3B, 0x3A, 0x0, 0x0, 0xB, 0xA, 
        0x0, 0x0, 0x1B, 0x1A, 0x0, 0x0, 0xB, 0xA}, {
        0x0, 0x0, 0x0, 0x0, 0xF7, 0xF6, 0xF4, 0xF4, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14, 
        0x0, 0x0, 0x0, 0x0, 0x37, 0x36, 0x34, 0x34, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14, 
        0x0, 0x0, 0x0, 0x0, 0x77, 0x76, 0x74, 0x74, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14, 
        0x0, 0x0, 0x0, 0x0, 0x37, 0x36, 0x34, 0x34, 
        0x0, 0x0, 0x0, 0x0, 0x17, 0x16, 0x14, 0x14}, {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0xEF, 0xEE, 0xEC, 0xEC, 0xE8, 0xE8, 0xE8, 0xE8, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x2F, 0x2E, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x6F, 0x6E, 0x6C, 0x6C, 0x68, 0x68, 0x68, 0x68, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x2F, 0x2E, 0x2C, 0x2C, 0x28, 0x28, 0x28, 0x28}, {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0xDF, 0xDE, 0xDC, 0xDC, 0xD8, 0xD8, 0xD8, 0xD8, 
        0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x5F, 0x5E, 0x5C, 0x5C, 0x58, 0x58, 0x58, 0x58, 
        0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50}, {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
        0xBF, 0xBE, 0xBC, 0xBC, 0xB8, 0xB8, 0xB8, 0xB8, 
        0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 
        0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 
        0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0}, {
        0x7F, 0x7E, 0x7C, 0x7C, 0x78, 0x78, 0x78, 0x78, 
        0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 
        0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
        0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 
        0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40}};

        //Rook Offset, Masks, and Boards
        static constexpr int RookOffset[64] = 
        {0x0, 0x1000, 0x1800, 0x2000, 
        0x2800, 0x3000, 0x3800, 0x4000, 
        0x5000, 0x5800, 0x5C00, 0x6000, 
        0x6400, 0x6800, 0x6C00, 0x7000, 
        0x7800, 0x8000, 0x8400, 0x8800, 
        0x8C00, 0x9000, 0x9400, 0x9800, 
        0xA000, 0xA800, 0xAC00, 0xB000, 
        0xB400, 0xB800, 0xBC00, 0xC000, 
        0xC800, 0xD000, 0xD400, 0xD800, 
        0xDC00, 0xE000, 0xE400, 0xE800, 
        0xF000, 0xF800, 0xFC00, 0x10000, 
        0x10400, 0x10800, 0x10C00, 0x11000, 
        0x11800, 0x12000, 0x12400, 0x12800, 
        0x12C00, 0x13000, 0x13400, 0x13800, 
        0x14000, 0x15000, 0x15800, 0x16000, 
        0x16800, 0x17000, 0x17800, 0x18000};
        static constexpr Bitboard RookMasks[64] = 
        {0x101010101017EULL, 0x202020202027CULL, 0x404040404047AULL, 0x8080808080876ULL, 
        0x1010101010106EULL, 0x2020202020205EULL, 0x4040404040403EULL, 0x8080808080807EULL, 
        0x1010101017E00ULL, 0x2020202027C00ULL, 0x4040404047A00ULL, 0x8080808087600ULL, 
        0x10101010106E00ULL, 0x20202020205E00ULL, 0x40404040403E00ULL, 0x80808080807E00ULL, 
        0x10101017E0100ULL, 0x20202027C0200ULL, 0x40404047A0400ULL, 0x8080808760800ULL, 
        0x101010106E1000ULL, 0x202020205E2000ULL, 0x404040403E4000ULL, 0x808080807E8000ULL, 
        0x101017E010100ULL, 0x202027C020200ULL, 0x404047A040400ULL, 0x8080876080800ULL, 
        0x1010106E101000ULL, 0x2020205E202000ULL, 0x4040403E404000ULL, 0x8080807E808000ULL, 
        0x1017E01010100ULL, 0x2027C02020200ULL, 0x4047A04040400ULL, 0x8087608080800ULL, 
        0x10106E10101000ULL, 0x20205E20202000ULL, 0x40403E40404000ULL, 0x80807E80808000ULL, 
        0x17E0101010100ULL, 0x27C0202020200ULL, 0x47A0404040400ULL, 0x8760808080800ULL, 
        0x106E1010101000ULL, 0x205E2020202000ULL, 0x403E4040404000ULL, 0x807E8080808000ULL, 
        0x7E010101010100ULL, 0x7C020202020200ULL, 0x7A040404040400ULL, 0x76080808080800ULL, 
        0x6E101010101000ULL, 0x5E202020202000ULL, 0x3E404040404000ULL, 0x7E808080808000ULL, 
        0x7E01010101010100ULL, 0x7C02020202020200ULL, 0x7A04040404040400ULL, 0x7608080808080800ULL, 
        0x6E10101010101000ULL, 0x5E20202020202000ULL, 0x3E40404040404000ULL, 0x7E80808080808000ULL};
        
        static Bitboard RookBoards[0x19000];

        static Bitboard hqRookAttack(int, Bitboard);
        static Bitboard rookAttack(int, Bitboard);

        //Bishop Offset, Masks, and Boards
        static constexpr int BishopOffset[64] = 
        {0x0, 0x40, 0x60, 0x80, 
        0xA0, 0xC0, 0xE0, 0x100, 
        0x140, 0x160, 0x180, 0x1A0, 
        0x1C0, 0x1E0, 0x200, 0x220, 
        0x240, 0x260, 0x280, 0x300, 
        0x380, 0x400, 0x480, 0x4A0, 
        0x4C0, 0x4E0, 0x500, 0x580, 
        0x780, 0x980, 0xA00, 0xA20, 
        0xA40, 0xA60, 0xA80, 0xB00, 
        0xD00, 0xF00, 0xF80, 0xFA0, 
        0xFC0, 0xFE0, 0x1000, 0x1080, 
        0x1100, 0x1180, 0x1200, 0x1220, 
        0x1240, 0x1260, 0x1280, 0x12A0, 
        0x12C0, 0x12E0, 0x1300, 0x1320, 
        0x1340, 0x1380, 0x13A0, 0x13C0, 
        0x13E0, 0x1400, 0x1420, 0x1440};
        static constexpr Bitboard BishopMasks[64] = 
        {0x40201008040200ULL, 0x402010080400ULL, 0x4020100A00ULL, 0x40221400ULL,
        0x2442800ULL, 0x204085000ULL, 0x20408102000ULL, 0x2040810204000ULL,
        0x20100804020000ULL, 0x40201008040000ULL, 0x4020100A0000ULL, 0x4022140000ULL,
        0x244280000ULL, 0x20408500000ULL, 0x2040810200000ULL, 0x4081020400000ULL,
        0x10080402000200ULL, 0x20100804000400ULL, 0x4020100A000A00ULL, 0x402214001400ULL,
        0x24428002800ULL, 0x2040850005000ULL, 0x4081020002000ULL, 0x8102040004000ULL,
        0x8040200020400ULL, 0x10080400040800ULL, 0x20100A000A1000ULL, 0x40221400142200ULL,
        0x2442800284400ULL, 0x4085000500800ULL, 0x8102000201000ULL, 0x10204000402000ULL, 
        0x4020002040800ULL, 0x8040004081000ULL, 0x100A000A102000ULL, 0x22140014224000ULL,
        0x44280028440200ULL, 0x8500050080400ULL, 0x10200020100800ULL, 0x20400040201000ULL,
        0x2000204081000ULL, 0x4000408102000ULL, 0xA000A10204000ULL, 0x14001422400000ULL,
        0x28002844020000ULL, 0x50005008040200ULL, 0x20002010080400ULL, 0x40004020100800ULL,
        0x20408102000ULL, 0x40810204000ULL, 0xA1020400000ULL, 0x142240000000ULL,
        0x284402000000ULL, 0x500804020000ULL, 0x201008040200ULL, 0x402010080400ULL,
        0x2040810204000ULL, 0x4081020400000ULL, 0xA102040000000ULL, 0x14224000000000ULL,
        0x28440200000000ULL, 0x50080402000000ULL, 0x20100804020000ULL, 0x40201008040200ULL};
        
        static Bitboard BishopBoards[0x1480];

        static Bitboard hqBishopAttack(int, Bitboard);
        static Bitboard bishopAttack(int, Bitboard);

        static void initSlideAttacks();

        Bitboard sides[2], pieces[6];
        bool toMove;

        uint8_t cf[2] = {7, 0};
        uint8_t crc[64] = 
            {8, 0, 0, 0, 12, 0, 0, 4,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0,
             2, 0, 0, 0, 3, 0, 0, 1};

        int thm, chm[128];
        uint8_t cr[128]; //last 4 bits q k Q K
        uint8_t ep[128];

        //zobrist history and keys

        //FEN-related parsing
        static constexpr char frchr[21] = 
        {'/', '1', '2', '3', '4', '5', '6', '7', '8',
            'k', 'q', 'r', 'b', 'n', 'p', 'K', 'Q', 'R', 'B', 'N', 'P'};

        static constexpr char cstrt[4] = 
        {'K', 'Q', 'k', 'q'};


        Bitboards();

        static void printAsBitboard(Bitboard);

        int pieceAt(int);

        //bool insufficientMaterial();

        void setStartPos();
        void empty();
        
        //int countReps(int);

        void print();

        //void beginZobristHash();

        void readFen(std::string);

};

/*
Function Definitions for Bitboards Class

TheTilted096, 12-19-24
Wilted Engine
*/

Bitboard Bitboards::RookBoards[0x19000];
Bitboard Bitboards::BishopBoards[0x1480];

Bitboards::Bitboards(){
    empty();
}

Bitboard Bitboards::hqRookAttack(int sq, Bitboard occ){
    Bitboard forward = occ & (0x0101010101010101ULL << (sq & 7));
    Bitboard reverse = __builtin_bswap64(forward);

    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));

    forward ^= __builtin_bswap64(reverse);
    forward &= (0x0101010101010101ULL << (sq & 7)); //forward is now vertical attacks

    //printAsBitboard(forward, std::cout);

    Bitboard horizontal = ((uint64_t)hlt[sq & 7]
        [((((0xFFULL << ((sq & 56))) & occ) >> (sq & 56)) >> 1) & 63]) << (sq & 56);

    //printAsBitboard(horizontal, std::cout);
    //printAsBitboard(forward | horizontal, std::cout);

    return (forward | horizontal);
}

Bitboard Bitboards::rookAttack(int sq, Bitboard occ){
    return RookBoards[RookOffset[sq] + _pext_u64(occ, RookMasks[sq])];
}

Bitboard Bitboards::hqBishopAttack(int sq, Bitboard occ){
    //Position::printAsBitboard(occ);
    Bitboard ldiag = 0x8040201008040201ULL;
    Bitboard rdiag = 0x102040810204080ULL;

    Bitboard sqfile = 0x101010101010101ULL << (sq & 7);
    
    Bitboard forward, reverse;
    //right diagonal
    int d = __builtin_ctzll(sqfile & rdiag) - sq;
    //std::cout << d << '\n';
    Bitboard mask = (d >= 0) ? (rdiag >> d) : (rdiag << -d);
    forward = occ & mask;
    reverse = __builtin_bswap64(forward);
    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));
    forward ^= __builtin_bswap64(reverse);
    forward &= mask;

    Bitboard result = forward; //save right diagonal attacks

    d = __builtin_ctzll(sqfile & ldiag) - sq;
    //std::cout << d << '\n';
    mask = (d >= 0) ? (ldiag >> d) : (ldiag << -d);
    forward = occ & mask;
    reverse = __builtin_bswap64(forward);
    forward -= 2 * (1ULL << sq);
    reverse -= 2 * (1ULL << (56 ^ sq));
    forward ^= __builtin_bswap64(reverse);
    forward &= mask;

    result |= forward;

    //Position::printAsBitboard(result);

    return result;
}

Bitboard Bitboards::bishopAttack(int sq, Bitboard occ){
    return BishopBoards[BishopOffset[sq] + _pext_u64(occ, BishopMasks[sq])];
}

void Bitboards::initSlideAttacks(){
    int pos = 0;
    Bitboard cons;

    const uint8_t RookBits[64] = 
        {12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12};

    //std::ofstream rt("rawTables.txt");

    for (int sq = 0; sq < 64; sq++){
        for (int idx = 0; idx < (1 << RookBits[sq]); idx++){
            cons = _pdep_u64(idx, RookMasks[sq]) | (1ULL << sq);
            RookBoards[pos] = hqRookAttack(sq, cons);
            //printAsBitboard(RookBoards[pos], rt);
            pos++;
        }
    }

    const uint8_t BishopBits[64] = 
        {6, 5, 5, 5, 5, 5, 5, 6, 
        5, 5, 5, 5, 5, 5, 5, 5, 
        5, 5, 7, 7, 7, 7, 5, 5, 
        5, 5, 7, 9, 9, 7, 5, 5, 
        5, 5, 7, 9, 9, 7, 5, 5, 
        5, 5, 7, 7, 7, 7, 5, 5, 
        5, 5, 5, 5, 5, 5, 5, 5, 
        6, 5, 5, 5, 5, 5, 5, 6};

    pos = 0;

    for (int sq = 0; sq < 64; sq++){
        for (int idx = 0; idx < (1 << BishopBits[sq]); idx++){
            cons = _pdep_u64(idx, BishopMasks[sq]) | (1ULL << sq);
            BishopBoards[pos] = hqBishopAttack(sq, cons);

            pos++;
        }
    }
}

void Bitboards::printAsBitboard(Bitboard board){
    for (int i = 0; i < 64; i++) {
        std::cout << (board & 1ULL);
        if ((i & 7) == 7) {
            std::cout << '\n';
        }
        board >>= 1;
    }
    std::cout << '\n';
}

int Bitboards::pieceAt(int sq){
    Bitboard sqbb = (1ULL << sq);
    for (int i = 0; i < 6; i++){
        if (pieces[i] & sqbb){
            return i;
        }
    }
    return 13; //in case of fail 
    //(but also en passant)
}

void Bitboards::setStartPos(){
    sides[0] = 0xFFFFULL;
    sides[1] = 0xFFFF000000000000ULL;

    pieces[0] = 0x1000000000000010ULL;
    pieces[1] = 0x800000000000008ULL;
    pieces[2] = 0x8100000000000081ULL;
    pieces[3] = 0x2400000000000024ULL;
    pieces[4] = 0x4200000000000042ULL;
    pieces[5] = 0xFF00000000FF00ULL;

    toMove = true;
    thm = 0;
    ep[0] = 255; //out of range en passant because there is none
    cr[0] = 15;    
}

void Bitboards::empty(){
    sides[0] = 0ULL; sides[1] = 0ULL;
    pieces[0] = 0ULL; pieces[1] = 0ULL; pieces[2] = 0ULL;
    pieces[3] = 0ULL; pieces[4] = 0ULL; pieces[5] = 0ULL;

    toMove = true;
    thm = 0;
    ep[0] = 255; //out of range en passant because there is none
    cr[0] = 0;    
}

void Bitboards::print(){
    std::cout << "sides[0] (black)\tsides[1] (white)\n";
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            std::cout << ((sides[0] >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << "\t\t";
        for (int j = 0; j < 8; j++){
            std::cout << ((sides[1] >> ((i << 3) + j)) & 1ULL);
        }
        std::cout << '\n';
    }

    std::cout << "\n\npieces[0]\tpieces[1]\tpieces[2]\tpieces[3]\tpieces[4]\tpieces[5]\n";
    for (int i = 0; i < 8; i++){
        for (int k = 0; k < 6; k++){
            for (int j = 0; j < 8; j++){
                std::cout << ((pieces[k] >> ((i << 3) + j)) & 1ULL);
            }
            std::cout << '\t';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    std::cout << "toMove: " << toMove << "\n\n";

    std::cout << "Half Move: " << chm[thm] << '\n';

    std::cout << "Castling: " << (int) cr[thm] << '\n';
    std::cout << "En Passant: " << (int) ep[thm] << "\n\n";
}

void Bitboards::readFen(std::string fen){
    //EXAMPLE: rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 moves aa
    empty();

    std::stringstream segs(fen);

    std::string feed;

    segs >> feed; //main information segment
    
    uint8_t ind, sq = 0;
    for (char cc : feed){
        for (ind = 0; ind < 21; ind++){ //use std::find?
            if (frchr[ind] == cc){
                break;
            }
        }

        if ((0 < ind) and (ind < 9)){
            sq += ind;
            continue;
        }
        if ((ind >= 9) and (ind < 15)){
            pieces[ind - 9] |= (1ULL << sq);
            sides[0] |= (1ULL << sq);
            sq++;
            continue;
        }
        if ((ind >= 15) and (ind < 21)){
            pieces[ind - 15] |= (1ULL << sq);
            sides[1] |= (1ULL << sq);
            sq++;
            continue;
        }
    }

    segs >> feed; //side to move
    toMove = feed[0] & 1; //'w' = 119 and 'b' = 98;
    
    segs >> feed; //castling rights 'KQkq' or something
    feed += ' ';
    ind = 0;

    //input total 1/2 move clock starting at 0th
    thm = 0;

    //might want to optimize this later
    cr[0] |= (cstrt[0] == feed[ind]);
    ind += (cstrt[0] == feed[ind]);

    cr[0] |= ((cstrt[1] == feed[ind]) << 1);
    ind += (cstrt[1] == feed[ind]);

    cr[0] |= ((cstrt[2] == feed[ind]) << 2);
    ind += (cstrt[2] == feed[ind]);

    cr[0] |= ((cstrt[3] == feed[ind]) << 3);
    ind += (cstrt[3] == feed[ind]);

    segs >> feed; //en passant square
    ep[0] = (feed != "-") ? ((feed[0] - 97) + 8 * (56 - feed[1])) : 255;

    //implement half move clock
    segs >> feed;
    chm[0] = std::stoi(feed); 
}

