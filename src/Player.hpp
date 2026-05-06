#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>
#include <string>
#include "Tile.hpp"
#include "Hand.hpp"
#include "ShantenCounter.hpp"

class Player {
protected:
    std::string name;
    int seat;               // 0: 東, 1: 南, 2: 西, 3: 北
    bool isDealer;          
    Hand hand;              
    std::vector<Tile> river; 
    std::vector<Meld> melds; 
    bool isRiichi;

public:
    Player(std::string n, int s);
    virtual ~Player() {} 

    // 共同狀態管理
    void setDealer(bool dealer) { isDealer = dealer; }
    bool getIsDealer() const { return isDealer; }
    int getSeat() const { return seat; }
    const std::vector<Tile>& getRiver() const { return river; }
    const Hand& getHand() const { return hand; }

    // 共同行為
    void drawTile(const Tile& tile);      
    Tile discardTile(const Tile& tile);   
    
    // polymorphism
    virtual Tile decideDiscard() = 0;

    void performChi(const std::vector<Tile>& combo, const Tile& discardedTile);
    void performPon(const Tile& discardedTile);
    void performKan(const Tile& discardedTile, MeldType kanType);

    void displayStatus() const;
};

// 沒邏輯的玩家 - 隨機丟牌
class RandomPlayer : public Player {
public:
    RandomPlayer(std::string n, int s) : Player(n, s) {}
    Tile decideDiscard() override; // 實作隨機丟牌
};

// 出階邏輯 - 最小化向聽數
class ShantenPlayer : public Player {
private:
    ShantenCounter sc; // 導入向聽數計算器

public:
    ShantenPlayer(std::string n, int s) : Player(n, s) {}

    // 打牌決策
    Tile decideDiscard() override;
};

#endif