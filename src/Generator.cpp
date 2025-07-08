// Generator Function Definition

#include "Generator.h"

Generator::Generator(){}

template <bool captureOnly> Count Generator::generate(MoveList& moveList){
    Count totalMoves = 0;

    Bitboard moveSet, pieces, captureSet;

    Bitboard occupancy = pos->occupied();
    Bitboard allies = pos->ours();
    Bitboard enemies = pos->theirs();

    Square origin, destination;
    Piece victimType;
    
    // Pawns

    pieces = pos->our(Pawn);
    Bitboard pawnTargets = enemies;

    if (pos->thisPassant() != XX){
        pawnTargets |= squareBitboard(pos->thisPassant());
    }

    Bitboard leftCaptures = (pieces & 0xFEFEFEFEFEFEFEFEULL) << 7; //mask off A file, capture diagonally down and left
    Bitboard rightCaptures = (pieces & 0x7F7F7F7F7F7F7F7FULL) << 9; //mask off H file, capture diagonally down and right

    if (pos->toMove){ //if it's white to move
        leftCaptures >>= 16; //white captures towards the lower bits, shift targets up 2 ranks
        rightCaptures >>= 16;
    }

    leftCaptures &= pawnTargets;
    rightCaptures &= pawnTargets;

    while (rightCaptures){
        destination = popLeastBit(rightCaptures);

        moveList[totalMoves].setFrom(static_cast<Square>(destination - 9 + (pos->toMove << 4)));
        moveList[totalMoves].setTo(destination);

        moveList[totalMoves].setMoving(Pawn);

        victimType = pos->pieceAt(destination);
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

        moveList[totalMoves].setFrom(static_cast<Square>(destination - 7 + (pos->toMove << 4)));
        moveList[totalMoves].setTo(destination);

        moveList[totalMoves].setMoving(Pawn);

        victimType = pos->pieceAt(destination);
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
        Bitboard pushes = ((pieces << 8) >> (pos->toMove << 4)) & ~occupancy;

        //double push destinations are a square past the single push, but they also must be on the 4th rank (relative)
        Bitboard doublePushes = ((pushes << 8) >> (pos->toMove << 4)) & ~occupancy & (0xFF000000ULL << (pos->toMove << 3));

        while (pushes){
            destination = popLeastBit(pushes);

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 8 + (pos->toMove << 4)));
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

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 16 + (pos->toMove << 5)));
            moveList[totalMoves].setTo(destination);
            moveList[totalMoves].setMoving(Pawn);
            moveList[totalMoves].setEnding(Pawn);

            moveList[totalMoves].setDoublePush();

            totalMoves++;
        }    
    }
 
    //Knights
    pieces = pos->our(Knight);
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
            moveList[totalMoves].setCaptured(pos->pieceAt(destination));
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
    pieces = pos->our(Bishop);
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
            moveList[totalMoves].setCaptured(pos->pieceAt(destination));
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
    pieces = pos->our(Rook);
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
            moveList[totalMoves].setCaptured(pos->pieceAt(destination));
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
    pieces = pos->our(Queen);
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
            moveList[totalMoves].setCaptured(pos->pieceAt(destination));
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
    origin = getLeastBit(pos->our(King));

    moveSet = Attacks::KingAttacks[origin] & ~allies;
    captureSet = moveSet & enemies;
    moveSet ^= captureSet;

    while (captureSet){
        destination = popLeastBit(captureSet);

        moveList[totalMoves].setFrom(origin);
        moveList[totalMoves].setTo(destination);

        //moveList[totalMoves].setMoving(King); King = 0
        moveList[totalMoves].setCaptured(pos->pieceAt(destination));
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

        uint8_t rights = pos->ourRights(); //castling
        //std::cout << "CR: " << (int)rights << '\n';
        if (rights){
            if (rights & 1){ //kingside
                if (!(pos->kingOccMask[pos->toMove] & occupancy)){
                    moveList[totalMoves].setFrom(origin);
                    moveList[totalMoves].setTo(Position::kingKingTo[pos->toMove]);

                    //moveList[totalMoves].setMoving(King);
                    //moveList[totalMoves].setEnding(King);

                    moveList[totalMoves].setKingCastle();
                    totalMoves++;
                }
            }

            if (rights & 2){ //queenside
                if (!(pos->queenOccMask[pos->toMove] & occupancy)){
                    moveList[totalMoves].setFrom(origin);
                    moveList[totalMoves].setTo(Position::queenKingTo[pos->toMove]);

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

template Count Generator::generate<false>(MoveList&);
template Count Generator::generate<true>(MoveList&);

Count Generator::countLegal(){
    MoveList ml;
    Count pl = generate<false>(ml);

    Count num = 0;

    for (Count i = 0; i < pl; i++){
        pos->makeMove(ml[i]);
        //std::cout << ml[i].toString() << '\n';
        if (pos->illegal()){
            pos->unmakeMove();
            //std::cout << "Legality Prune\n";
            continue;
        }

        num++;

        pos->unmakeMove();
    }

    return num;
}

Move Generator::unalgebraic(std::string str){
    MoveList testers = {};
    Count c = generateMoves(testers);

    for (Count i = 0; i < c; i++){
        if (pos->moveName(testers[i]) == str){
            return testers[i];
        }
    }

    return Move::Invalid;
}




