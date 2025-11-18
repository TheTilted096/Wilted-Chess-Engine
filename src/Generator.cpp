// Generator Function Definition

#include "Generator.h"

// Optimization macros
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define FORCE_INLINE __attribute__((always_inline)) inline

Generator::Generator(){}

template <Piece p, bool cf> FORCE_INLINE void Generator::spreadMoves(MoveList& ml, Count& tm, const Square& o, Bitboard& ds){
    Square k;

    while (LIKELY(ds)){
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

    const Bitboard occupancy = pos->occupied();
    const Bitboard allies = pos->ours();
    const Bitboard enemies = pos->theirs();

    Square origin, destination;
    const Square kingsq = getLeastBit(pos->our(King));
    Piece victimType;

    // Compute threats more efficiently
    Bitboard threats = 0ULL;
    {
        // Pawn threats
        Bitboard pawnPieces = pos->their(Pawn);
        Bitboard leftCaptures = (pawnPieces & 0xFEFEFEFEFEFEFEFEULL) << 7;
        Bitboard rightCaptures = (pawnPieces & 0x7F7F7F7F7F7F7F7FULL) << 9;

        if constexpr (!stm){
            leftCaptures >>= 16;
            rightCaptures >>= 16;
        }

        threats = leftCaptures | rightCaptures;

        // Knight threats
        Bitboard knightPieces = pos->their(Knight);
        while (LIKELY(knightPieces)){
            origin = popLeastBit(knightPieces);
            threats |= Attacks::KnightAttacks[origin];
        }

        // Sliding piece threats (computed without king)
        const Bitboard noKingOcc = occupancy ^ pos->our(King);

        Bitboard diagPieces = pos->diagonalPieces() & enemies;
        while (LIKELY(diagPieces)){
            origin = popLeastBit(diagPieces);
            threats |= Attacks::bishopAttacks(origin, noKingOcc);
        }

        Bitboard straightPieces = pos->straightPieces() & enemies;
        while (LIKELY(straightPieces)){
            origin = popLeastBit(straightPieces);
            threats |= Attacks::rookAttacks(origin, noKingOcc);
        }

        threats |= Attacks::KingAttacks[getLeastBit(pos->their(King))];
    }

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

    // Compute pinned pieces more efficiently
    Bitboard pinned = 0ULL;
    {
        Bitboard enemyDiag = enemies & pos->diagonalPieces();
        Bitboard enemyStraight = enemies & pos->straightPieces();

        pieces = (Attacks::bishopAttacks(kingsq, enemyDiag) & enemyDiag) |
                 (Attacks::rookAttacks(kingsq, enemyStraight) & enemyStraight);

        while (LIKELY(pieces)){
            origin = popLeastBit(pieces);
            Bitboard between = occupancy & Attacks::RayBetween[origin][kingsq];

            // Check if exactly one piece between and it's ours
            if (UNLIKELY(between && !(between & (between - 1)) && ((between & allies) == between))){
                pinned |= between;
            }
        }
    }

    Bitboard checkmask = ~0ULL;
    if constexpr (numChecks == 1){
        checkmask = checkers | Attacks::RayBetween[getLeastBit(checkers)][kingsq];
    }

    captureSet = enemies;
    Bitboard epcheckmask = checkmask;

    const Square epSquare = pos->thisPassant();
    if (UNLIKELY(epSquare != XX)){
        captureSet |= squareBitboard(epSquare);
        epcheckmask |= (((checkers & pos->their(Pawn)) << 8) >> (stm << 4));
    }

    // Pawn captures - compute both left and right more efficiently
    const Bitboard ourPawns = pos->our(Pawn);
    Bitboard leftPawns, rightPawns;

    if constexpr (stm){
        leftPawns = ourPawns & (~pinned | Attacks::LeftDiags[kingsq]);
        rightPawns = ourPawns & (~pinned | Attacks::RightDiags[kingsq]);
    } else {
        leftPawns = ourPawns & (~pinned | Attacks::RightDiags[kingsq]);
        rightPawns = ourPawns & (~pinned | Attacks::LeftDiags[kingsq]);
    }

    Bitboard left = (leftPawns & 0xFEFEFEFEFEFEFEFEULL) << 7;
    Bitboard right = (rightPawns & 0x7F7F7F7F7F7F7F7FULL) << 9;

    if constexpr (stm){
        left >>= 16;
        right >>= 16;
    }

    left &= captureSet & epcheckmask;
    right &= captureSet & epcheckmask;

    // Process right pawn captures
    while (LIKELY(right)){
        destination = popLeastBit(right);
        origin = static_cast<Square>(destination - 9 + (stm << 4));

        moveList[totalMoves].setFrom(origin);
        moveList[totalMoves].setTo(destination);
        moveList[totalMoves].setMoving(Pawn);

        victimType = pos->pieceAt(destination);
        if (UNLIKELY(victimType == None)){ // en passant
            if (UNLIKELY(squareBitboard(kingsq) & (0xFF00000000ULL >> (stm << 3)))){
                Bitboard postocc = occupancy ^ (0x3ULL << origin);
                if (UNLIKELY(pos->straightPieces() & enemies & Attacks::rookAttacks(kingsq, postocc))){
                    continue;
                }
            }
            moveList[totalMoves].setEpCaps();
            victimType = Pawn;
        }

        moveList[totalMoves].setCaptured(victimType);

        if (UNLIKELY(squareBitboard(destination) & 0xFF000000000000FFULL)){
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

    // Process left pawn captures
    while (LIKELY(left)){
        destination = popLeastBit(left);
        origin = static_cast<Square>(destination - 7 + (stm << 4));

        moveList[totalMoves].setFrom(origin);
        moveList[totalMoves].setTo(destination);
        moveList[totalMoves].setMoving(Pawn);

        victimType = pos->pieceAt(destination);
        if (UNLIKELY(victimType == None)){ // en passant
            if (UNLIKELY(squareBitboard(kingsq) & (0xFF00000000ULL >> (stm << 3)))){
                Bitboard postocc = occupancy ^ (0x3ULL << (origin - 1));
                if (UNLIKELY(pos->straightPieces() & enemies & Attacks::rookAttacks(kingsq, postocc))){
                    continue;
                }
            }
            moveList[totalMoves].setEpCaps();
            victimType = Pawn;
        }

        moveList[totalMoves].setCaptured(victimType);

        if (UNLIKELY(squareBitboard(destination) & 0xFF000000000000FFULL)){
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

    // Pawn pushes (non-captures)
    if constexpr (!captureOnly){
        pieces = ourPawns & (~pinned | (0x0101010101010101ULL << (kingsq & 7)));
        Bitboard pushes = ((pieces << 8) >> (stm << 4)) & ~occupancy;
        Bitboard doublePushes = ((pushes << 8) >> (stm << 4)) & ~occupancy & (0xFF000000ULL << (stm << 3));

        pushes &= checkmask;
        doublePushes &= checkmask;

        while (LIKELY(pushes)){
            destination = popLeastBit(pushes);

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 8 + (stm << 4)));
            moveList[totalMoves].setTo(destination);
            moveList[totalMoves].setMoving(Pawn);

            if (UNLIKELY(squareBitboard(destination) & 0xFF000000000000FFULL)){
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

        while (LIKELY(doublePushes)){
            destination = popLeastBit(doublePushes);

            moveList[totalMoves].setFrom(static_cast<Square>(destination - 16 + (pos->toMove << 5)));
            moveList[totalMoves].setTo(destination);
            moveList[totalMoves].setMoving(Pawn);
            moveList[totalMoves].setEnding(Pawn);
            moveList[totalMoves].setDoublePush();

            totalMoves++;
        }
    }

    // Knights - unpinned only
    pieces = pos->our(Knight) & ~pinned;
    while (LIKELY(pieces)){
        origin = popLeastBit(pieces);

        moveSet = Attacks::KnightAttacks[origin] & ~allies & checkmask;
        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Knight, true>(moveList, totalMoves, origin, captureSet);

        if constexpr (!captureOnly){
            spreadMoves<Knight, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    // Bishops
    pieces = pos->our(Bishop);
    while (LIKELY(pieces)){
        origin = popLeastBit(pieces);

        moveSet = Attacks::bishopAttacks(origin, occupancy) & ~allies & checkmask;
        if (UNLIKELY((pinned >> origin) & 1)){
            moveSet &= Attacks::RayIncluding[origin][kingsq];
        }

        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Bishop, true>(moveList, totalMoves, origin, captureSet);
        if constexpr (!captureOnly){
            spreadMoves<Bishop, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    // Rooks
    pieces = pos->our(Rook);
    while (LIKELY(pieces)){
        origin = popLeastBit(pieces);

        moveSet = Attacks::rookAttacks(origin, occupancy) & ~allies & checkmask;
        if (UNLIKELY((pinned >> origin) & 1)){
            moveSet &= Attacks::RayIncluding[origin][kingsq];
        }

        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Rook, true>(moveList, totalMoves, origin, captureSet);
        if constexpr (!captureOnly){
            spreadMoves<Rook, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    // Queens
    pieces = pos->our(Queen);
    while (LIKELY(pieces)){
        origin = popLeastBit(pieces);

        moveSet = Attacks::queenAttacks(origin, occupancy) & ~allies & checkmask;
        if (UNLIKELY((pinned >> origin) & 1)){
            moveSet &= Attacks::RayIncluding[origin][kingsq];
        }

        captureSet = moveSet & enemies;
        moveSet ^= captureSet;

        spreadMoves<Queen, true>(moveList, totalMoves, origin, captureSet);
        if constexpr (!captureOnly){
            spreadMoves<Queen, false>(moveList, totalMoves, origin, moveSet);
        }
    }

    // King moves
    moveSet = Attacks::KingAttacks[kingsq] & ~allies & ~threats;
    captureSet = moveSet & enemies;
    moveSet ^= captureSet;

    spreadMoves<King, true>(moveList, totalMoves, kingsq, captureSet);

    if constexpr (!captureOnly){
        spreadMoves<King, false>(moveList, totalMoves, kingsq, moveSet);

        // Castling (only when not in check)
        if constexpr (numChecks == 0){
            const uint8_t rights = pos->ourRights();

            if (UNLIKELY(rights)){
                if (UNLIKELY(rights & 1)){ // Kingside
                    if (!(pos->kingOccMask[stm] & occupancy)){
                        if (!(pos->kingSafeMask[stm] & threats)){
                            if (!pos->isFRC || !((pinned >> pos->kingRookFrom[stm]) & 1)){
                                moveList[totalMoves].setFrom(kingsq);
                                moveList[totalMoves].setTo(Position::kingKingTo[stm]);
                                moveList[totalMoves].setKingCastle();
                                totalMoves++;
                            }
                        }
                    }
                }

                if (UNLIKELY(rights & 2)){ // Queenside
                    if (!(pos->queenOccMask[stm] & occupancy)){
                        if (!(pos->queenSafeMask[stm] & threats)){
                            if (!pos->isFRC || !((pinned >> pos->queenRookFrom[stm]) & 1)){
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




