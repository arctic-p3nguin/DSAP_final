#include <iostream>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include "GameManager.hpp"

using namespace std;


// 測試跑完一局
int main() {
    // 1. Initialize random seed to ensure different game results each run
    srand(static_cast<unsigned int>(time(nullptr)));

    cout << "===============================================" << endl;
    cout << "   NTU IM Mahjong AI Project - Prototype       " << endl;
    cout << "   Validation: 3 Random AI and 1 Shanten AI Battle Simulation    " << endl;
    cout << "===============================================" << endl;

    // 2. Initialize Game Engine
    GameManager gm;
    
    // 3. Start Game: Deck initialization, shuffling, and dealing
    gm.startGame();

    int turnCounter = 0;
    bool gameRunning = true;

    // todo: 之後做完吃碰槓要回來改輸出邏輯
    while (gameRunning && turnCounter < 500) { 

        if (turnCounter % 4 == 0) {
            cout << "\n=== Start of Round " << (turnCounter / 4) + 1 << " ===" << endl;
            gm.printGameState();
        }

        cout << "[Turn " << setw(3) << turnCounter + 1 << "] ";
        gameRunning = gm.nextStep();
        
        turnCounter++;
    }

    // 5. Final Result
    cout << "\n===============================================" << endl;
    if (turnCounter >= 500) {
        cout << "System Notice: Maximum turn limit reached. Terminating." << endl;
    } else {
        cout << "Game Finished. Total turns executed: " << turnCounter << endl;
    }
    cout << "===============================================" << endl;

    return 0;
}

