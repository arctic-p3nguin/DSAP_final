#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include "Player.hpp"
#include "Tile.hpp"
#include "Rules.hpp"

class GameManager {
private:
    std::vector<Tile> deck;          // 牌堆 (136張)
    std::vector<Player*> players;      // 四位玩家
    int currentTurn;                 // 當前手番 (0-3)
    Rules rules;                     // 規則判定

    void initDeck();                 // 產生 136 張牌
    void shuffleDeck();              // 洗牌
    void dealTiles();         // 開局發牌

public:
    GameManager();
    void startGame();                // 初始化並開始遊戲
    bool nextStep();                 // 執行摸切 (回傳 false 代表遊戲結束)
    void printGameState() const;     // 驗證用
    bool isDeckEmpty() const { return deck.empty(); }
};

#endif