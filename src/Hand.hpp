#ifndef HAND_HPP
#define HAND_HPP

#include <vector>
#include <algorithm> 
#include "Tile.hpp"

// note: 暗槓會加到meld 但不算是副露 不影響門前清
enum class MeldType {
    Chi,
    Pon,
    Ankan, 
    Minkan, 
    Kakan  
};

struct Meld {
    MeldType type; 
    std::vector<Tile> tiles;
};

class Hand {
private:
    std::vector<Tile> concealedTiles;  
    std::vector<Meld> exposedMelds; 

public:
    void addTile(Tile t);             
    Tile discardTile(int type, int val, int id); 
    void sortHand();                  
    void addMeld(Meld m);             

    const std::vector<Tile>& getConcealed() const;
    const std::vector<Meld>& getMelds() const;
};

#endif