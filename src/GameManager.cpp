#include "GameManager.hpp"
#include <iostream>

using namespace std;

GameManager::GameManager() : currentTurn(0) {
    // 建立四位玩家，並預設 0 號為莊家
    players.push_back(new RandomPlayer("Player 0", 0));
    players.push_back(new RandomPlayer("Player 1", 1));
    players.push_back(new RandomPlayer("Player 2", 2));
    players.push_back(new ShantenPlayer("Player 3", 3));
    
    players[0]->setDealer(true);
}

void GameManager::initDeck() {
    deck.clear();
    // 萬 (0)、索 (1)、筒 (2) 各 9 種牌，字 (3) 有 7 種
    for (int t = 0; t < 4; ++t) {
        int maxVal = (t == 3) ? 7 : 9;
        for (int v = 0; v < maxVal; ++v) {
            for (int i = 0; i < 4; ++i) { // 每種牌有 4 張 (ID 0~3)
                deck.emplace_back(t, v, i, false);
            }
        }
    }
}

void GameManager::shuffleDeck() {
    random_device rd;
    mt19937 g(rd());
    shuffle(deck.begin(), deck.end(), g);
}

void GameManager::dealTiles() {
    // 每人發 13 張
    for (int round = 0; round < 13; ++round) {
        for (auto& p : players) {
            p->drawTile(deck.back());
            deck.pop_back();
        }
    }
}

void GameManager::startGame() {
    initDeck();
    shuffleDeck();
    dealTiles();
    cout << "--- Game Started ---" << endl;
}

bool GameManager::nextStep() {
    if (deck.empty()) {
        cout << "Exhaustive Draw! (Deck is empty)" << endl; 
        return false;
    }

    Player* p = players[currentTurn];
    
    // 1. Draw
    Tile drawn = deck.back();
    deck.pop_back();
    p->drawTile(drawn); 
    cout << "Player " << currentTurn << " draws a tile." << endl;

    if (currentTurn == 3) {
        ShantenCounter sc; 
        int currentShanten = sc.calculate(p->getHand().getConcealed(), p->getHand().getMelds());
        cout << ">>> [AI Analysis] Player 3 after drawing: Shanten = " << currentShanten << endl;
    }

    // 2. Win check
    if (rules.isWin(p->getHand().getConcealed())) { 
        cout << "!!! Player " << currentTurn << " Win (Tsumo)! !!!" << endl;
        p->displayStatus();
        return false;
    }

    // 3. AI Decision
    Tile toDiscard = p->decideDiscard(); 
    p->discardTile(toDiscard); 
    
    cout << "Player " << currentTurn << " discards Tile[Type: " 
         << toDiscard.getType() << ", Val: " << toDiscard.getValue() << "]" << endl;
    
    // 4. Ron check
    for (int i = 0; i < 4; ++i) {
        if (i == currentTurn) continue; 

        Player* otherPlayer = players[i];
        vector<Tile> simulatedHand = otherPlayer->getHand().getConcealed();
        simulatedHand.push_back(toDiscard);

        if (rules.isWin(simulatedHand)) {
            cout << "!!! Player " << i << " Win (Ron) on Player " << currentTurn << "'s discard! !!!" << endl;
            cout << "Winning Tile: [Type: " << toDiscard.getType() << ", Val: " << toDiscard.getValue() << "]" << endl;
            return false; 
        }
    }

    // 5. Turn switch
    currentTurn = (currentTurn + 1) % 4;
    return true;
}

void GameManager::printGameState() const {
    cout << "--------------------------------------" << endl;
    cout << "Deck Remaining: " << deck.size() << " | Next Turn: Player " << currentTurn << endl;
    cout << "--------------------------------------" << endl;
}