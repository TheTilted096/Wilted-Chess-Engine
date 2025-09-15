// Class Definition for Generator

#pragma once

#include "Position.h"

using MoveList = std::array<Move, MOVELIST_SIZE>;

class Generator{
    public:
        Position* pos;

        Generator();

        void assign(Position* pp){ pos = pp; }

        //Count countLegal();

        template <Piece, bool> void spreadMoves(MoveList&, Count&, const Square&, Bitboard&);

        template <Color, bool, Count> Count genCheckCase(MoveList&, const Bitboard&);
        template <Color, bool> Count genColorCase(MoveList&);

        template <bool> Count generate(MoveList&);

        Count generateMoves(MoveList& ml){ return generate<false>(ml); }
        Count generateCaptures(MoveList& ml){ return generate<true>(ml); }

        Move unalgebraic(std::string);
};