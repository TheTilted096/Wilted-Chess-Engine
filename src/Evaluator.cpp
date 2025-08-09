// Evaluation Function Definitions

#include "Evaluator.h"

Evaluator::Evaluator(){
    for (Piece i = Queen; i < None; i++){
        for (Square j = a8; j < XX; j++){
            midpst[i][j] += material[i];
            endpst[i][j] += material[i];
        }
    }
}

Score Evaluator::judge() const{
    int m = (midScores[pos->toMove] - midScores[!pos->toMove]) * gamePhase;
    int e = (endScores[pos->toMove] - endScores[!pos->toMove]) * (totalPhase - gamePhase);

    return (m + e) / totalPhase;
}

Score Evaluator::refresh(){
    midScores[White] = 0;
    midScores[Black] = 0;

    endScores[White] = 0;
    endScores[Black] = 0;

    gamePhase = 0;

    Bitboard pcs;
    Square sq;

    for (Piece i = King; i < None; i++){
        pcs = pos->those(White, i);

        while (pcs){
            sq = popLeastBit(pcs);
            midScores[White] += midpst[i][sq];
            endScores[White] += endpst[i][sq];
            //std::cout << "Added White " << (int)i << " at " << (int)sq << '\n';
        }

        pcs = pos->those(Black, i);

        while (pcs){
            sq = popLeastBit(pcs);
            midScores[Black] += midpst[i][flip(sq)]; // Black flips
            endScores[Black] += endpst[i][flip(sq)];
            //std::cout << "Added Black " << (int)i << " at " << (int)flip(sq) << '\n';
        }
    }

    for (Piece p = Queen; p < None; p++){
        gamePhase += phases[p] * std::popcount<Bitboard>(pos->any(p));
    }

    return judge();
}

void Evaluator::doMove(const Move& m){ //called immediately after pos->makeMove(m)
    Square start = m.from();
    Square end = m.to();
    Piece typei = m.moving();
    Piece typef = m.ending();

    Piece victim = m.captured();

    Color us = flip(pos->toMove); //side that made the move, could be dangerous when attempting lazy updates

    if (victim){
        bool passant = m.epCapture();

        Square target = static_cast<Square>(end + passant * ((us << 4) - 8));
        midScores[!us] -= midpst[victim][target ^ (56 * us)];
        endScores[!us] -= endpst[victim][target ^ (56 * us)];
        //std::cout << "Access midpst " << (int)victim << ' ' << (int)(target ^ (56 * us)) << '\n';

        gamePhase -= phases[victim]; // subtract captured piece
    }

    Square starte = static_cast<Square>(start ^ (56 * !us));
    Square ende = static_cast<Square>(end ^ (56 * !us));

    midScores[us] += (midpst[typef][ende] - midpst[typei][starte]);
    endScores[us] += (endpst[typef][ende] - endpst[typei][starte]);
    //std::cout << "Access midpst " << (int)typef << ' ' << (int)ende << " - then - midpst " << (int)typei << ' ' << (int)starte << '\n';

    gamePhase += (phases[typef] - phases[typei]); // in case of promotion, add/sub the phases of the moving piece

    if (m.castling()){
        if (m.kingCastle()){ //kingside castle 
            start = pos->kingRookFrom[us];
            end = pos->kingRookTo[us];
        } else {
            start = pos->queenRookFrom[us];
            end = pos->queenRookTo[us];
        }

        starte = static_cast<Square>(start ^ (56 * !us));
        ende = static_cast<Square>(end ^ (56 * !us));

        midScores[us] += (midpst[Rook][ende] - midpst[Rook][starte]);
        endScores[us] += (endpst[Rook][ende] - endpst[Rook][starte]);
        //std::cout << "Access midpst " << (int)Rook << ' ' << (int)ende << " - then - midpst " << (int)Rook << ' ' << (int)starte << '\n';
    }
}

void Evaluator::undoMove(const Move& m){
    Square start = m.from();
    Square end = m.to();
    Piece typei = m.moving();
    Piece typef = m.ending();

    Piece victim = m.captured();

    Color us = pos->toMove; //might be dangerous in lazy update
    //here assumed undoMove is called immediately after pos->unmakeMove()

    if (victim){
        bool passant = m.epCapture();

        Square target = static_cast<Square>(end + passant * ((us << 4) - 8));
        midScores[!us] += midpst[victim][target ^ (56 * us)];
        endScores[!us] += endpst[victim][target ^ (56 * us)];

        gamePhase += phases[victim];
    }

    Square starte = static_cast<Square>(start ^ (56 * !us));
    Square ende = static_cast<Square>(end ^ (56 * !us));

    midScores[us] -= (midpst[typef][ende] - midpst[typei][starte]);
    endScores[us] -= (endpst[typef][ende] - endpst[typei][starte]);

    gamePhase -= (phases[typef] - phases[typei]); // in case of promotion, add/sub the phases of the moving piece

    if (m.castling()){
        if (m.kingCastle()){ //kingside castle 
            start = pos->kingRookFrom[us];
            end = pos->kingRookTo[us];
        } else {
            start = pos->queenRookFrom[us];
            end = pos->queenRookTo[us];
        }

        starte = static_cast<Square>(start ^ (56 * !us));
        ende = static_cast<Square>(end ^ (56 * !us));

        midScores[us] -= (midpst[Rook][ende] - midpst[Rook][starte]);
        endScores[us] -= (endpst[Rook][ende] - endpst[Rook][starte]);
    }
}







