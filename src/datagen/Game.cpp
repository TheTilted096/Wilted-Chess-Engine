// Function Definitions of Game Class

#include "Game.h"

uint64_t Game::maxSoftNodes;
uint32_t Game::openOffset;
uint32_t Game::endOffset;
uint32_t Game::maxHalf;

std::ofstream Game::ofile;
std::ofstream Game::hofile;

bool Game::adjudicate(){
    Position& mp = players[turn].mainpos;
    Generator& mg = players[turn].maingen;

    if (mg.countLegal() == 0){
        if (mp.isChecked(turn)){
            result = !turn;
            verdict = turn ? "Black" : "White";
            verdict += " Mates";
        } else {
            verdict = "Stalemate";
            result = 0.5;
        }

        return true;
    }

    if (mp.repetitions(0) > 2){
        result = 0.5;
        verdict = "Threefold Repetition";
        return true;
    }

    if (mp.insufficient()){
        result = 0.5;
        verdict = "Insufficient Material";
        return true;
    }

    if (mp.sinceReset() >= 100){
        result = 0.5;
        verdict = "Fifty Moves Rule";
        return true;
    }

    return false;
}   

void Game::play(std::string opening){
    { //small thing that finds the first side to move
    int i;
    for (i = 0; opening[i] != ' '; i++){}
    turn = static_cast<Color>(opening[i + 1] & 1);
    }

    clock = 0;

    positions.clear();
    scores.clear();

    players[Black].mainpos.setFRC();
    players[White].mainpos.setFRC();

    players[Black].newGame();
    players[White].newGame();

    players[Black].mainpos.readFen(opening);
    players[White].mainpos.readFen(opening);

    Move lastmove;
    Score sc;

    while (true){
        if (adjudicate()){
            ending = players[turn].mainpos.makeFen();
            break;
        }

        sc = players[turn].goSoftly(maxSoftNodes) * (2 * turn - 1);
        lastmove = players[turn].bestMove();

        //if (!players[turn].mainpos.isChecked(turn) and !(lastmove.captured() or lastmove.promoted())
        //        and (players[turn].mainpos.sinceReset() <= maxHalf)){
            
            positions.push_back(players[turn].mainpos.makeFen());
            scores.push_back(sc);
        //}

        players[Black].mainpos.makeMove(lastmove);
        players[White].mainpos.makeMove(lastmove);
        if (lastmove.resets()){
            players[Black].mainpos.forget();
            players[White].mainpos.forget();
        }

        clock++;

        turn = flip(turn);
    }
}

void Game::report(uint32_t id){
    hofile << "GAME " << id << '\n';
    hofile << "RESULT: " << result << '\n';
    hofile << "REASON: " << verdict << '\n';
    hofile << "START: " << positions[0] << '\n';
    for (int i = openOffset; i < std::max(0, (int)(clock - endOffset)); i++){
        hofile << "D: " << positions[i] << " | " << scores[i] << " | " << result << '\n';
        ofile << positions[i] << " | " << scores[i] << " | " << result << '\n';
    }
    hofile << "FINAL: " << ending << '\n';
}








