// Wrapper Class for Perft

#include "Generator.h"

class Performer{
    public:
        Generator gen;
        Position* posptr;

        Performer(Position*);

        template <bool> uint64_t perft(const Depth&);

        void run(const Depth& d);
};

Performer::Performer(Position* pp){
    gen.assign(pp);
    posptr = pp;
}

template <bool rootNode> uint64_t Performer::perft(const Depth& depth){
    if (depth == 0){
        return 1ULL;
    }

    uint64_t nodes = 0ULL;
    MoveList mlist = {};

    //gen.posptr->print();

    Count plegal = gen.generateMoves(mlist);
    //std::cout << (int)plegal << " generated\n";

    for (Count i = 0; i < plegal; i++){
        posptr->makeMove(mlist[i]);
        //std::cout << mlist[i].toString() << '\n';

        if (gen.illegal()){
            //std::cout << "illegal prune\n";
            posptr->unmakeMove();
            continue;
        }

        uint64_t next = perft<false>(depth - 1);

        posptr->unmakeMove();

        if constexpr (rootNode){
            std::cout << posptr->moveName(mlist[i]) << ": " << /*mlist[i].info << " : " << */next << '\n';
        }

        nodes += next;
    }

    if constexpr (rootNode){
        std::cout << '\n' << nodes << " nodes\n";
    }

    return nodes;
}

void Performer::run(const Depth& depth){
    auto start = std::chrono::steady_clock::now();

    uint64_t all = perft<true>(depth);

    auto end = std::chrono::steady_clock::now();

    double dur = 1 + std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    int nps = (all / dur) * 1000000;
    std::cout << nps << " nps\n\n";
}