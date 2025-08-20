// Function Definitions of Game Class

#include "Game.h"

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

    host.command = "position fen " + opening + " moves ";
}









