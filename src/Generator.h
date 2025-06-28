// Class Definition for Generator

#include "Attacks.h"
#include "Position.h"

class Generator{
    public:
        Position* pos;

        Generator();

        void assign(Position* pp){ pos = pp; }

        bool isAttacked(const Square&, const Color&) const;
        bool isChecked(const Color&) const;

        bool illegal() const;

        Count countLegal();

        template <bool> Count generate(std::array<Move, 128>&);

        Count generateMoves(std::array<Move, 128>& ml){ return generate<false>(ml); }
        Count generateCaptures(std::array<Move, 128>& ml){ return generate<true>(ml); }

        Move unalgebraic(std::string);
};