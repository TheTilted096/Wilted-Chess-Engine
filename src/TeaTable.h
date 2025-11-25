// Class Definition for TTentry and Transposition Table

#pragma once

#include "Move.h"

enum class NodeType : uint8_t{ Exact = 1, Cut, All };

class Teacup{ //TTentry
    public:
        uint64_t data; //move 0-31, score 32-47, depth 48-53, nodetype 54-55, generation 56-63
        uint64_t hash; //use high multiplication trick later

        Score eScore(Index ply = 0);
        NodeType enType();
        Depth eDepth();
        Hash eHash();
        Move eMove();
        uint8_t eGen();

        Teacup();
        void update(Score, NodeType, Depth, Hash, Move, Index, uint8_t);
        void reset();
};

class TeaTable{ //Transposition Table
    public:
        Teacup* storage;
        std::size_t capacity = 32 * 1048576 / sizeof(Teacup); //32 MiB

        static constexpr std::size_t maxCapacity = 128 * 1024 * 1024 / sizeof(Teacup); //128 MiB
        static constexpr std::size_t minCapacity = 1 * 1024 * 1024 / sizeof(Teacup); //1 MiB;
        uint8_t generation = 0;

        TeaTable();
        
        void resize(std::size_t); //resizes to specified MiB
        void clear();

        Teacup& operator[](std::size_t ind){
            return storage[ind];
        }

        Teacup& at(std::size_t ind){ return storage[ind]; }

        std::size_t find(const Hash& h){ return (h % capacity); }
        Teacup& probe(const Hash& h){ return storage[find(h)]; }

        ~TeaTable();  
};