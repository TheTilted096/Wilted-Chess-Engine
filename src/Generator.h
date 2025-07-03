// Class Definition for Generator

#pragma once

#include "Attacks.h"
#include "Position.h"

constexpr std::size_t MOVELIST_SIZE = 128;
using MoveList = std::array<Move, MOVELIST_SIZE>;

class Generator{
    public:
        Position* pos;

        Generator();

        void assign(Position* pp){ pos = pp; }

        bool isAttacked(const Square&, const Color&) const;
        bool isChecked(const Color&) const;

        bool illegal() const;

        Count countLegal();

        template <bool> Count generate(MoveList&);

        Count generateMoves(MoveList& ml){ return generate<false>(ml); }
        Count generateCaptures(MoveList& ml){ return generate<true>(ml); }

        Move unalgebraic(std::string);
};