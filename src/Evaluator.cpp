// Evaluation Function Definitions

#include "Evaluator.h"

Evaluator::Evaluator(){
    for (Piece i = Queen; i < None; i++){
        for (Square j = a8; j < XX; j++){
            midpst[i][j] += material[i];
        }
    }
}

Score Evaluator::judge() const{
    return (midScores[pos->toMove] - midScores[!pos->toMove]);
}

Score Evaluator::refresh(){
    midScores[White] = 0;
    midScores[Black] = 0;

    Bitboard pcs;
    Square sq;

    for (Piece i = King; i < None; i++){
        pcs = pos->those(White, i);

        while (pcs){
            sq = popLeastBit(pcs);
            midScores[White] += midpst[i][sq];
            //std::cout << "Added White " << (int)i << " at " << (int)sq << '\n';
        }

        pcs = pos->those(Black, i);

        while (pcs){
            sq = popLeastBit(pcs);
            midScores[Black] += midpst[i][flip(sq)]; // Black flips
            //std::cout << "Added Black " << (int)i << " at " << (int)flip(sq) << '\n';
        }
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
        //std::cout << "Access midpst " << (int)victim << ' ' << (int)(target ^ (56 * us)) << '\n';

    }

    Square starte = static_cast<Square>(start ^ (56 * !us));
    Square ende = static_cast<Square>(end ^ (56 * !us));

    midScores[us] += (midpst[typef][ende] - midpst[typei][starte]);
    //std::cout << "Access midpst " << (int)typef << ' ' << (int)ende << " - then - midpst " << (int)typei << ' ' << (int)starte << '\n';

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
    }

    Square starte = static_cast<Square>(start ^ (56 * !us));
    Square ende = static_cast<Square>(end ^ (56 * !us));

    midScores[us] -= (midpst[typef][ende] - midpst[typei][starte]);

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
    }
}







