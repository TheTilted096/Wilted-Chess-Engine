// Class Definition for TTentry and Transposition Table

#pragma once

#include "Move.h"

enum class NodeType : uint8_t{ Exact = 1, Cut, All };

class Teacup{ //TTentry
    public:
        uint64_t data; //move 0-31, score 32-47, depth 48-53, nodetype 54-55
        uint64_t hash; //use high multiplication trick later

        Score eScore();
        NodeType enType();
        Depth eDepth();
        Hash eHash();
        Move eMove();

        Teacup();
        void update(Score, NodeType, Depth, Hash, Move);
        void reset();
};

class TeaTable{ //Transposition Table
    public:
        Teacup* storage;
        std::size_t capacity = 32 * 1048576 / sizeof(Teacup); //32 MiB

        static constexpr std::size_t maxCapacity = 128 * 1024 * 1024 / sizeof(Teacup); //128 MiB
        static constexpr std::size_t minCapacity = 1 * 1024 * 1024 / sizeof(Teacup); //1 MiB;

        TeaTable();
        
        void resize(std::size_t); //resizes to specified MiB
        void clear();

        Teacup& operator[](std::size_t ind){
            return storage[ind];
        }

        ~TeaTable();  
};