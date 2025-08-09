// Wrapper Class for Perft

#pragma once

#include "Generator.h"

class Performer{
    public:
        Generator gen;
        Position* posptr;

        Performer(Position*);

        template <bool> uint64_t perft(const Depth&);

        void run(const Depth& d);
};