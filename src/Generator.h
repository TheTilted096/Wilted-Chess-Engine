// Class Definition for Generator

#pragma once

#include "Attacks.h"
#include "Position.h"

using Actions = std::array<Move, 128>;

class Generator{
    public:
        Position* pos;

        Generator();

        void assign(Position* pp){ pos = pp; }

        bool isAttacked(const Square&, const Color&) const;
        bool isChecked(const Color&) const;

        bool illegal() const;

        Count countLegal();

        template <bool> Count generate(Actions&);

        Count generateMoves(Actions& ml){ return generate<false>(ml); }
        Count generateCaptures(Actions& ml){ return generate<true>(ml); }

        Move unalgebraic(std::string);
};