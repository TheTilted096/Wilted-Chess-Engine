// Generator Function Definition

#include "Generator.h"

Generator::Generator(){}

template <Piece p, bool cf> void Generator::spreadMoves(MoveList& ml, Count& tm, const Square& o, Bitboard& ds){
    Square k;

    while (ds){
        k = popLeastBit(ds);

        ml[tm].setFrom(o);
        ml[tm].setTo(k);

        ml[tm].setMoving(p);
        if constexpr (cf){ ml[tm].setCaptured(pos->pieceAt(k)); }
        ml[tm].setEnding(p);

        tm++;
    }
}

template <Color stm, bool captureOnly, Count numChecks> Count Generator::genCheckCase(MoveList& moveList, const Bitboard& checkers){
    Count totalMoves = 0;

    Bitboard moveSet, pieces, captureSet;

    Bitboard occupancy = pos->occupied();
    Bitboard allies = pos->ours();
    Bitboard enemies = pos->theirs();

    Square origin, destination, kingsq = getLeastBit(pos->our(King));
    Piece victimType;

    Bitboard threats = [&](){
        Bitboard t = 0ULL;

        pieces = pos->their(Pawn);

        Bitboard leftCaptures = (pieces & 0xFEFEFEFEFEFEFEFEULL) << 7; //mask off A file, capture diagonally down and left
        Bitboard rightCaptures = (pieces & 0x7F7F7F7F7F7F7F7FULL) << 9; //mask off H file, capture diagonally down and right

        if constexpr (!stm){ //if it's white to move
            leftCaptures >>= 16; //white captures towards the lower bits, shift targets up 2 ranks
            rightCaptures >>= 16;
        }

        t |= leftCaptures;
        t |= rightCaptures;

        pieces = pos->their(Knight);
        while (pieces){
            origin = popLeastBit(pieces);
            t |= Attacks::KnightAttacks[origin];
        }

        Bitboard noKingOcc = occupancy ^ pos->our(King);
        pieces = pos->diagonalPieces() & enemies;
        while (pieces){
            origin = popLeastBit(pieces);
            t |= Attacks::bishopAttacks(origin, noKingOcc);
        }

        pieces = pos->straightPieces() & enemies;
        while (pieces){
            origin = popLeastBit(pieces);
            t |= Attacks::rookAttacks(origin, noKingOcc);
        }

        t |= Attacks::KingAttacks[getLeastBit(pos->their(King))];

        return t;
    }();

    if constexpr (numChecks > 1){ // double check
        moveSet = Attacks::KingAttacks[kingsq] & ~allies & ~threats;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<King, true>(moveList, totalMoves, kingsq, captureSet);

        if constexpr (!captureOnly){
            spreadMoves<King, false>(moveList, totalMoves, kingsq, moveSet);
        }

        return totalMoves;
    }

    Bitboard pinned = [&](){
        Bitboard p = 0ULL;

        Bitboard shared;

        shared = enemies & pos->diagonalPieces();
        pieces = Attacks::bishopAttacks(kingsq, shared) & shared; // enemy diagonal pieces

        shared = enemies & pos->straightPieces();
        pieces |= Attacks::rookAttacks(kingsq, shared) & shared; // enemy straight pieces
        while (pieces){
            origin = popLeastBit(pieces);
            shared = occupancy & Attacks::RayBetween[origin][kingsq];

            bool s = shared and !(shared & (shared - 1)) and ((shared & allies) == shared);
            if (s){ p |= shared; }
        }

        return p;
    }();

    Bitboard checkmask = ~0ULL;
    if constexpr (numChecks == 1){ // if in check, valid blocking moves are the ray and the checker itself.
        checkmask = checkers | Attacks::RayBetween[getLeastBit(checkers)][kingsq];
    }

    captureSet = enemies;
    Bitboard epcheckmask = checkmask;
    
    if (pos->thisPassant() != XX){
        captureSet |= squareBitboard(pos->thisPassant());

        epcheckmask |= (((checkers & pos->their(Pawn)) << 8) >> (stm << 4)); // pawn checkers and ep
    }

    pieces = pos->our(Pawn); // calculate valid left capturing pawns
    if constexpr (stm){
        pieces &= (~pinned | Attacks::LeftDiags[kingsq]);
    } else {
        pieces &= (~pinned | Attacks::RightDiags[kingsq]);
    }

    Bitboard left = (pieces & 0xFEFEFEFEFEFEFEFEULL) << 7; //mask off A file, capture diagonally down and left

    pieces = pos->our(Pawn); // calculate valid right capturing pawns
    if constexpr (stm){
        pieces &= (~pinned | Attacks::RightDiags[kingsq]);
    } else {
        pieces &= (~pinned | Attacks::LeftDiags[kingsq]);
    }

    Bitboard right = (pieces & 0x7F7F7F7F7F7F7F7FULL) << 9; //mask off H file, capture diagonally down and right

    if constexpr (stm){ //if it's white to move
        left >>= 16; //white captures towards the lower bits, shift targets up 2 ranks
        right >>= 16;
    }

    left &= captureSet;
    right &= captureSet;    

    left &= epcheckmask;
    right &= epcheckmask;

    while (right){
        destination = popLeastBit(right);

        origin = static_cast<Square>(destination - 9 + (stm << 4));

        moveList[totalMoves].setFrom(origin);    
        moveList[totalMoves].setTo(destination);

        moveList[totalMoves].setMoving(Pawn);

        victimType = pos->pieceAt(destination);
        if (victimType == None){ // en passant
            if (squareBitboard(kingsq) & (0xFF00000000ULL >> (stm << 3))){ //king is on the 4th rank
                Bitboard postocc = occupancy ^ (0x3ULL << origin);
                if (pos->straightPieces() & enemies & Attacks::rookAttacks(kingsq, postocc)){
                // determine if there are any horizontal pieces attacking the king after rank occupancy is adjusted
                    continue;
                }   
            }

            moveList[totalMoves].setEpCaps();
            victimType = Pawn;
        }

        moveList[totalMoves].setCaptured(victimType);

        if (squareBitboard(destination) & 0xFF000000000000FFULL){
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

    while (left){
        destination = popLeastBit(left);

        origin = static_cast<Square>(destination - 7 + (stm << 4));

        moveList[totalMoves].setFrom(origin);    
        moveList[totalMoves].setTo(destination);

        moveList[totalMoves].setMoving(Pawn);

        victimType = pos->pieceAt(destination);
        if (victimType == None){ // en passant
            if (squareBitboard(kingsq) & (0xFF00000000ULL >> (stm << 3))){ //king is on the 4th rank
                Bitboard postocc = occupancy ^ (0x3ULL << (origin - 1));
                if (pos->straightPieces() & enemies & Attacks::rookAttacks(kingsq, postocc)){
                    
                // determine if there are any horizontal pieces attacking the king after rank occupancy is adjusted
                    continue;
                }   
            }

            moveList[totalMoves].setEpCaps();
            victimType = Pawn;
        }

        moveList[totalMoves].setCaptured(victimType);

        if (squareBitboard(destination) & 0xFF000000000000FFULL){
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

    pieces = pos->our(Pawn) & (~pinned | (0x0101010101010101ULL << (kingsq & 7)));
    if constexpr (!captureOnly){
        Bitboard pushes = ((pieces << 8) >> (stm << 4)) & ~occupancy;
        //double push destinations are a square past the single push, but they also must be on the 4th rank (relative)
        Bitboard doublePushes = ((pushes << 8) >> (stm << 4)) & ~occupancy & (0xFF000000ULL << (stm << 3));

        pushes &= checkmask;
        doublePushes &= checkmask;

        while (pushes){
            destination = popLeastBit(pushes);

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 8 + (stm << 4)));
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
    pieces = pos->our(Knight) & ~pinned;
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::KnightAttacks[origin] & ~allies & checkmask;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Knight, true>(moveList, totalMoves, origin, captureSet);

        if constexpr (!captureOnly){
            spreadMoves<Knight, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    pieces = pos->our(Bishop);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::bishopAttacks(origin, occupancy) & ~allies & checkmask;
        if ((pinned >> origin) & 1){ // if pinned
            moveSet &= Attacks::RayIncluding[origin][kingsq];
        }

        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Bishop, true>(moveList, totalMoves, origin, captureSet);
        if constexpr (!captureOnly){
            spreadMoves<Bishop, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    pieces = pos->our(Rook);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::rookAttacks(origin, occupancy) & ~allies & checkmask;
        if ((pinned >> origin) & 1){ // if pinned
            moveSet &= Attacks::RayIncluding[origin][kingsq];
        }

        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Rook, true>(moveList, totalMoves, origin, captureSet);
        if constexpr (!captureOnly){
            spreadMoves<Rook, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    pieces = pos->our(Queen);
    while (pieces){
        origin = popLeastBit(pieces);

        moveSet = Attacks::queenAttacks(origin, occupancy) & ~allies & checkmask;
        if ((pinned >> origin) & 1){ // if pinned
            moveSet &= Attacks::RayIncluding[origin][kingsq];
        }

        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Queen, true>(moveList, totalMoves, origin, captureSet);
        if constexpr (!captureOnly){
            spreadMoves<Queen, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    moveSet = Attacks::KingAttacks[kingsq] & ~allies & ~threats;
    captureSet = moveSet & enemies;
    moveSet ^= captureSet;

    spreadMoves<King, true>(moveList, totalMoves, kingsq, captureSet);

    if constexpr (!captureOnly){
        spreadMoves<King, false>(moveList, totalMoves, kingsq, moveSet);
    
        if constexpr (numChecks == 0){ // not in check
            uint8_t rights = pos->ourRights();

            if (rights){
                if (rights & 1){ //Kingside
                    if (!(pos->kingOccMask[stm] & occupancy)){ // not occupied
                        if (!(pos->kingSafeMask[stm] & threats)){ // safe
                            if (!pos->isFRC or (pos->isFRC and !((pinned >> pos->kingRookFrom[stm]) & 1))){ // rook isn't pinned
                                moveList[totalMoves].setFrom(kingsq);
                                moveList[totalMoves].setTo(Position::kingKingTo[stm]);

                                moveList[totalMoves].setKingCastle();
                                totalMoves++;
                            }
                        }
                    }
                }

                if (rights & 2){ // queenside
                    if (!(pos->queenOccMask[stm] & occupancy)){
                        if (!(pos->queenSafeMask[stm] & threats)){
                            if (!pos->isFRC or (pos->isFRC and !((pinned >> pos->queenRookFrom[stm]) & 1))){
                                moveList[totalMoves].setFrom(kingsq);
                                moveList[totalMoves].setTo(Position::queenKingTo[stm]);
                                
                                moveList[totalMoves].setQueenCastle();
                                totalMoves++;
                            }
                        }
                    }
                }
            }

        }
    
    }

    return totalMoves;
}

template <Color stm, bool captureOnly> Count Generator::genColorCase(MoveList& ml){
    Bitboard chk = pos->isChecked(stm);
    switch (std::popcount<Bitboard>(chk)){
        case 0:
            return genCheckCase<stm, captureOnly, 0>(ml, 0ULL);
        case 1:
            return genCheckCase<stm, captureOnly, 1>(ml, chk);
        case 2:
            return genCheckCase<stm, captureOnly, 2>(ml, chk);
        default:
            __builtin_unreachable();
    }
}

template <bool captureOnly> Count Generator::generate(MoveList& moveList){
    if (pos->toMove){
        return genColorCase<White, captureOnly>(moveList);
    }    

    return genColorCase<Black, captureOnly>(moveList);
}

template Count Generator::generate<false>(MoveList&);
template Count Generator::generate<true>(MoveList&);

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




