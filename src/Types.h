// Type Definitions for Wilted Engine
#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>

using Bitboard = uint64_t;
using Hash = uint64_t;

enum Piece : uint8_t{King, Queen, Rook, Bishop, Knight, Pawn, None};

inline Piece operator++(Piece& p, int){
    Piece q = p;
    p = static_cast<Piece>(p + 1);
    return q;
}

inline Piece operator--(Piece& p, int){
    Piece q = p;
    p = static_cast<Piece>(p - 1);
    return q;
}

enum Color : bool{Black, White};

inline Color flip(const Color& c){
    return static_cast<Color>(c ^ 1);
}

enum Square : uint8_t{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6, 
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, XX
};

constexpr Square operator+=(Square& s, const uint8_t& a){
    s = static_cast<Square>(s + a);
    return s;
}

constexpr Square operator++(Square& s, int){
    Square t = s;
    s += 1;
    return t;
}

inline Square flip(const Square& s){
    return static_cast<Square>(s ^ 56);
}

using Index = uint8_t;
using Count = uint8_t;
using Depth = uint8_t;

using Score = int16_t;

constexpr std::size_t MOVELIST_SIZE = 128;

constexpr Score SCORE_INF = 21000;

constexpr Score VICTORY = 20000;
constexpr Score DEFEAT = -20000;
constexpr Score DRAW = 0;

constexpr Score MIN_VICTORY = 19000;
constexpr Score MIN_DEFEAT = -19000;

constexpr Index MAX_PLY = 48; //maximum PV/search length

constexpr int16_t QUIET_HISTORY_LIM = 16384;

constexpr Count MAX_THREADS = 8;