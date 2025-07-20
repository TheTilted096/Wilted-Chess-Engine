// Function Definitions for the TT

#include "TeaTable.h"

// TT entry definitions

Teacup::Teacup(){ reset(); }

Score Teacup::eScore(Index ply){
    Score sc = static_cast<Score>((data >> 32) & 0xFFFFULL);

    if (sc > MIN_VICTORY){
        return sc - ply;
    }
    if (sc < MIN_DEFEAT){
        return sc + ply;
    }
    return sc;
}

NodeType Teacup::enType(){
    return static_cast<NodeType>(data >> 54);
}

Depth Teacup::eDepth(){
    return static_cast<Depth>((data >> 48) & 0x3FULL);
}

Move Teacup::eMove(){
    return data & 0xFFFFFFFFULL;
}

Hash Teacup::eHash(){
    return hash;
}

void Teacup::reset(){
    data = 0ULL;
    hash = 0ULL;
}

void Teacup::update(Score s, NodeType nt, Depth d, Hash h, Move m, Index ply){
    data = m.info;
    if (s > MIN_VICTORY){ s += ply; }
    if (s < MIN_DEFEAT){ s -= ply; }

    data |= (static_cast<uint64_t>(s) << 32); //do the mate score thing later
    data |= (static_cast<uint64_t>(d) << 48);
    data |= (static_cast<uint64_t>(nt) << 54);

    hash = h;
}

// Transposition Table Definitions

TeaTable::TeaTable(){
    storage = new Teacup[capacity];
    clear();
}

void TeaTable::resize(std::size_t c){
    delete[] storage;
    capacity = c * 1024 * 1024 / sizeof(Teacup);
    storage = new Teacup[capacity];
}

void TeaTable::clear(){
    for (std::size_t i = 0; i < capacity; i++){
        storage[i].reset();
    }
}

TeaTable::~TeaTable(){
    delete[] storage;
}

