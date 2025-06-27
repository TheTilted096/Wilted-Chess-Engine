// Function Definitions for Generator

#include "Generator.h"

Generator::Generator(){

} //nothing

bool Generator::isAttacked(const Square& sq, const Color& c) const{
    Bitboard checkers;
    Moment now = posptr->thisMoment();

    checkers = Attacks::PawnAttacks[!c][sq] & now.those(c, Pawn);
    if (checkers){ return true; }

    checkers = Attacks::KnightAttacks[sq] & now.those(c, Knight);
    if (checkers){ return true; }

    checkers = Attacks::KingAttacks[sq] & now.those(c, King);
    if (checkers){ return true; }

    Bitboard occ = now.occupied();
    Bitboard army = now.sides[c];
    checkers = Attacks::rookAttacks(sq, occ) & now.straightPieces() & army;
    if (checkers){ return true; }

    checkers = Attacks::bishopAttacks(sq, occ) & now.diagonalPieces() & army;
    return !!checkers;
}

inline bool Generator::isChecked(const Color& c) const{ //is c in check
    Moment now = posptr->thisMoment();
    Square k = getLeastBit(now.those(c, King));

    return isAttacked(k, flip(c)); //is the king square attacked by the other side
}

bool Generator::illegalPos() const{ 
    Color us = posptr->stm();

    uint8_t cat = posptr->lastPlayed().castling();
    if (cat){ //Kingside = 1, Queenside = 2, choose the relevant one
        Bitboard extra = (cat - 1) ? posptr->queenSafeMask[!us] : posptr->kingSafeMask[!us];
        Square s;
        while (extra){
            s = popLeastBit(extra);
            if (isAttacked(s, us)){ //stm attacks a square nstm castled through
                return true;
            }
        }

        return false;
    }

    return isChecked(flip(us)); 
} //nstm is in check

template <bool captureOnly> Count Generator::generate(std::array<Move, 128>& moveList){
    Count totalMoves = 0;

    Bitboard moveSet, pieces, captureSet;

    Moment now = posptr->thisMoment();

    Bitboard occupancy = now.occupied();
    Bitboard allies = now.ours();
    Bitboard enemies = now.theirs();

    Square origin, destination;
    Piece victimType;

    //Pawns

    pieces = now.our(Pawn);

    Bitboard pawnTargets = enemies;

    if (now.enPassant != XX){
        pawnTargets |= squareBitboard(now.enPassant);
    }

    Bitboard leftCaptures = (pieces & 0xFEFEFEFEFEFEFEFEULL) << 7; //mask off A file, capture diagonally down and left
    Bitboard rightCaptures = (pieces & 0x7F7F7F7F7F7F7F7FULL) << 9; //mask off H file, capture diagonally down and right

    if (now.toMove){ //if it's white to move
        leftCaptures >>= 16; //white captures towards the lower bits, shift targets up 2 ranks
        rightCaptures >>= 16;
    }

    leftCaptures &= pawnTargets;
    rightCaptures &= pawnTargets;

    while (rightCaptures){
        destination = popLeastBit(rightCaptures);

        moveList[totalMoves].setFrom(static_cast<Square>(destination - 9 + (now.toMove << 4)));
        moveList[totalMoves].setTo(destination);

        moveList[totalMoves].setMoving(Pawn);

        victimType = now.pieceAt(destination);
        if (victimType == None){ //Empty capture must be en passant
            moveList[totalMoves].setEpCaps();
            victimType = Pawn;
        } 

        moveList[totalMoves].setCaptured(victimType);

        if (squareBitboard(destination) & 0xFF000000000000FFULL){ //promotion, destination is on a final rank
            moveList[totalMoves].setPromote();
            Move temp = moveList[totalMoves];

            moveList[totalMoves].setEnding(Queen);
            totalMoves++;

            moveList[totalMoves] = temp;
            moveList[totalMoves].setEnding(Rook);
            totalMoves++;

            moveList[totalMoves] = temp;
            moveList[totalMoves].setEnding(Bishop);
            totalMoves++;

            moveList[totalMoves] = temp;
            moveList[totalMoves].setEnding(Knight);
        } else {
            moveList[totalMoves].setEnding(Pawn);
        }

        totalMoves++;
    }

    while (leftCaptures){
        destination = popLeastBit(leftCaptures);

        moveList[totalMoves].setFrom(static_cast<Square>(destination - 7 + (now.toMove << 4)));
        moveList[totalMoves].setTo(destination);

        moveList[totalMoves].setMoving(Pawn);

        victimType = now.pieceAt(destination);
        if (victimType == None){
            moveList[totalMoves].setEpCaps();
            victimType = Pawn;
        } 

        moveList[totalMoves].setCaptured(victimType);

        if (squareBitboard(destination) & 0xFF000000000000FFULL){ //promotion, destination is on a final rank
            moveList[totalMoves].setPromote();
            Move temp = moveList[totalMoves];

            moveList[totalMoves].setEnding(Queen);
            totalMoves++;

            moveList[totalMoves] = temp;
            moveList[totalMoves].setEnding(Rook);
            totalMoves++;

            moveList[totalMoves] = temp;
            moveList[totalMoves].setEnding(Bishop);
            totalMoves++;

            moveList[totalMoves] = temp;
            moveList[totalMoves].setEnding(Knight);
        } else {
            moveList[totalMoves].setEnding(Pawn);
        }

        totalMoves++;
    }

    if constexpr (!captureOnly){
        Bitboard pushes = ((pieces << 8) >> (now.toMove << 4)) & ~occupancy;

        //double push destinations are a square past the single push, but they also must be on the 4th rank (relative)
        Bitboard doublePushes = ((pushes << 8) >> (now.toMove << 4)) & ~occupancy & (0xFF000000ULL << (now.toMove << 3));

        while (pushes){
            destination = popLeastBit(pushes);

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 8 + (now.toMove << 4)));
            moveList[totalMoves].setTo(destination);

            moveList[totalMoves].setMoving(Pawn);

            if (squareBitboard(destination) & 0xFF000000000000FFULL){ //promotion, destination is on a final rank
                moveList[totalMoves].setPromote();
                Move temp = moveList[totalMoves];

                moveList[totalMoves].setEnding(Queen);
                totalMoves++;

                moveList[totalMoves] = temp;
                moveList[totalMoves].setEnding(Rook);
                totalMoves++;

                moveList[totalMoves] = temp;
                moveList[totalMoves].setEnding(Bishop);
                totalMoves++;

                moveList[totalMoves] = temp;
                moveList[totalMoves].setEnding(Knight);
            } else {
                moveList[totalMoves].setEnding(Pawn);
            }

            totalMoves++;
        }
    
        while (doublePushes){
            destination = popLeastBit(doublePushes);

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 16 + (now.toMove << 5)));
            moveList[totalMoves].setTo(destination);
            moveList[totalMoves].setMoving(Pawn);
            moveList[totalMoves].setEnding(Pawn);

            moveList[totalMoves].setDoublePush();

            totalMoves++;
        }    
    }

    //Knights
    pieces = now.our(Knight);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::KnightAttacks[origin] & ~allies;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        while (captureSet){
            destination = popLeastBit(captureSet);

            moveList[totalMoves].setFrom(origin);
            moveList[totalMoves].setTo(destination);

            moveList[totalMoves].setMoving(Knight);
            moveList[totalMoves].setCaptured(now.pieceAt(destination));
            moveList[totalMoves].setEnding(Knight);

            totalMoves++;
        }

        if constexpr (!captureOnly){
            while (moveSet){
                destination = popLeastBit(moveSet);

                moveList[totalMoves].setFrom(origin);
                moveList[totalMoves].setTo(destination);

                moveList[totalMoves].setMoving(Knight);
                moveList[totalMoves].setEnding(Knight);

                totalMoves++;
            }
        }
    }

    //Bishops
    pieces = now.our(Bishop);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::bishopAttacks(origin, occupancy) & ~allies;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        while (captureSet){
            destination = popLeastBit(captureSet);

            moveList[totalMoves].setFrom(origin);
            moveList[totalMoves].setTo(destination);

            moveList[totalMoves].setMoving(Bishop);
            moveList[totalMoves].setCaptured(now.pieceAt(destination));
            moveList[totalMoves].setEnding(Bishop);

            totalMoves++;
        }

        if constexpr (!captureOnly){
            while (moveSet){
                destination = popLeastBit(moveSet);

                moveList[totalMoves].setFrom(origin);
                moveList[totalMoves].setTo(destination);

                moveList[totalMoves].setMoving(Bishop);
                moveList[totalMoves].setEnding(Bishop);

                totalMoves++;
            }
        }
    }

    //Rooks
    pieces = now.our(Rook);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::rookAttacks(origin, occupancy) & ~allies;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        while (captureSet){
            destination = popLeastBit(captureSet);

            moveList[totalMoves].setFrom(origin);
            moveList[totalMoves].setTo(destination);

            moveList[totalMoves].setMoving(Rook);
            moveList[totalMoves].setCaptured(now.pieceAt(destination));
            moveList[totalMoves].setEnding(Rook);

            totalMoves++;
        }

        if constexpr (!captureOnly){
            while (moveSet){
                destination = popLeastBit(moveSet);

                moveList[totalMoves].setFrom(origin);
                moveList[totalMoves].setTo(destination);

                moveList[totalMoves].setMoving(Rook);
                moveList[totalMoves].setEnding(Rook);

                totalMoves++;
            }
        }
    }

    //Queens
    pieces = now.our(Queen);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::queenAttacks(origin, occupancy) & ~allies;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        while (captureSet){
            destination = popLeastBit(captureSet);

            moveList[totalMoves].setFrom(origin);
            moveList[totalMoves].setTo(destination);

            moveList[totalMoves].setMoving(Queen);
            moveList[totalMoves].setCaptured(now.pieceAt(destination));
            moveList[totalMoves].setEnding(Queen);

            totalMoves++;
        }

        if constexpr (!captureOnly){
            while (moveSet){
                destination = popLeastBit(moveSet);

                moveList[totalMoves].setFrom(origin);
                moveList[totalMoves].setTo(destination);

                moveList[totalMoves].setMoving(Queen);
                moveList[totalMoves].setEnding(Queen);

                totalMoves++;
            }
        }
    }

    //Kings
    origin = getLeastBit(now.our(King));

    moveSet = Attacks::KingAttacks[origin] & ~allies;
    captureSet = moveSet & enemies;
    moveSet ^= captureSet;

    while (captureSet){
        destination = popLeastBit(captureSet);

        moveList[totalMoves].setFrom(origin);
        moveList[totalMoves].setTo(destination);

        //moveList[totalMoves].setMoving(King); King = 0
        moveList[totalMoves].setCaptured(now.pieceAt(destination));
        //moveList[totalMoves].setEnding(King);

        totalMoves++;
    }

    if constexpr (!captureOnly){
        while (moveSet){
            destination = popLeastBit(moveSet);

            moveList[totalMoves].setFrom(origin);
            moveList[totalMoves].setTo(destination);

            //moveList[totalMoves].setMoving(King);
            //moveList[totalMoves].setEnding(King);

            totalMoves++;
        }

        uint8_t rights = now.ourRights(); //castling
        //std::cout << "CR: " << (int)rights << '\n';
        if (rights){
            if (rights & 1){ //kingside
                if (!(posptr->kingOccMask[now.toMove] & occupancy)){
                    moveList[totalMoves].setFrom(origin);
                    moveList[totalMoves].setTo(Position::kingKingTo[now.toMove]);

                    //moveList[totalMoves].setMoving(King);
                    //moveList[totalMoves].setEnding(King);

                    moveList[totalMoves].setKingCastle();
                    totalMoves++;
                }
            }

            if (rights & 2){ //queenside
                if (!(posptr->queenOccMask[now.toMove] & occupancy)){
                    moveList[totalMoves].setFrom(origin);
                    moveList[totalMoves].setTo(Position::queenKingTo[now.toMove]);

                    //moveList[totalMoves].setMoving(King);
                    //moveList[totalMoves].setEnding(King);

                    moveList[totalMoves].setQueenCastle();
                    totalMoves++;
                }
            }
        }
    }

    return totalMoves;

}

Count Generator::countLegal(){
    std::array<Move, 128> ml;
    Count pl = generate<false>(ml);

    Count num = 0;

    for (Count i = 0; i < pl; i++){
        posptr->makeMove(ml[i]);
        //std::cout << ml[i].toString() << '\n';
        if (illegalPos()){
            posptr->unmakeMove();
            //std::cout << "Legality Prune\n";
            continue;
        }

        num++;

        posptr->unmakeMove();
    }

    return num;
}

Move Generator::unalgebraic(std::string str){
    std::array<Move, 128> testers = {};
    Count c = generateMoves(testers);

    for (Count i = 0; i < c; i++){
        if (testers[i].toString() == str){
            return testers[i];
        }
    }

    return Move::Invalid;
}

