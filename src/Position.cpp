// Function Definitions for Position

#include "Position.h"

Position::Position(){
    empty();

    plays[0] = ~0U;

    for (Moment& m : moments){ //let them know the castling strings
        m.setCastleChars(&castleStrings);
    }
}

void Position::makeMove(const Move& m){
    Square start = m.from();
    Square end = m.to();

    Piece typei = m.moving();
    Piece typef = m.ending();

    Piece victim = m.captured();

    Color us = moments[clock].toMove;

    clock++;

    plays[clock] = m;

    moments[clock].pieces = moments[clock - 1].pieces;
    moments[clock].sides = moments[clock - 1].sides;
    moments[clock].toMove = flip(moments[clock - 1].toMove);

    moments[clock].castleRights = moments[clock - 1].castleRights;

    moments[clock].hash = moments[clock - 1].hash ^ Zobrist::turnKey;

    if (victim){
        bool passant = m.epCapture();
        //the target square, which is the end square depending on ep
        Square target = static_cast<Square>(static_cast<uint8_t>(end) + passant * ((us << 4) - 8));

        moments[clock].pieces[victim] ^= squareBitboard(target);
        moments[clock].sides[!us] ^= squareBitboard(target);
        
        moments[clock].hash ^= Zobrist::pieceKeys[!us][victim][target];
    }

    moments[clock].sides[us] ^= (squareBitboard(start) | squareBitboard(end)); //move the pieces
    moments[clock].pieces[typei] ^= squareBitboard(start);
    moments[clock].pieces[typef] ^= squareBitboard(end);

    moments[clock].hash ^= Zobrist::pieceKeys[us][typei][start];
    moments[clock].hash ^= Zobrist::pieceKeys[us][typef][end];

    if (m.doublePushed()){ //double push
        moments[clock].enPassant = static_cast<Square>(static_cast<uint8_t>(end) - 8 + (us << 4));
        moments[clock].hash ^= Zobrist::passantKeys[end & 7];
    } else {
        moments[clock].enPassant = XX;
    }

    Square lastPassant = moments[clock - 1].enPassant; //clear any en passant from before
    if (lastPassant != XX){
        moments[clock].hash ^= Zobrist::passantKeys[lastPassant & 7];
    }

    if (victim or (typei == Pawn)){ //half move reset
        moments[clock].halfMoves = 0;
    } else {
        moments[clock].halfMoves = moments[clock - 1].halfMoves + 1;
    }

    if (moments[clock].castleRights){ //if can castle
        uint8_t change = rightsChange[start] | rightsChange[end];
        if (change){
            moments[clock].castleRights &= ~change;
            moments[clock].hash ^= Zobrist::castleKeys[moments[clock - 1].castleRights]; //replace zobrist key
            moments[clock].hash ^= Zobrist::castleKeys[moments[clock].castleRights];
        }
    }

    if (m.castling()){
        if (m.kingCastle()){ //kingside castle 
            start = kingRookFrom[us];
            end = kingRookTo[us];

            moments[clock].sides[us] ^= (squareBitboard(start) | squareBitboard(end));
            moments[clock].pieces[Rook] ^= (squareBitboard(start) | squareBitboard(end));

            moments[clock].hash ^= Zobrist::pieceKeys[us][Rook][start];
            moments[clock].hash ^= Zobrist::pieceKeys[us][Rook][end];
        } else { //queenside castle
            start = queenRookFrom[us];
            end = queenRookTo[us];

            moments[clock].sides[us] ^= (squareBitboard(start) | squareBitboard(end));
            moments[clock].pieces[Rook] ^= (squareBitboard(start) | squareBitboard(end));

            moments[clock].hash ^= Zobrist::pieceKeys[us][Rook][start];
            moments[clock].hash ^= Zobrist::pieceKeys[us][Rook][end];
        }
    }
}

void Position::unmakeMove(){
    clock--;
}







