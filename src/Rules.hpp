#ifndef RULES_HPP
#define RULES_HPP

#include <vector>
#include "Tile.hpp"
#include "Hand.hpp"

using namespace std;


class Rules {
public:
    vector<vector<Tile>> getChiCombinations(const vector<Tile>& concealedTiles, const Tile& discardedTile);
    bool isWin(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds);
    vector<Tile> getWaitingTiles(vector<Tile> concealedTiles, const vector<Meld>& exposedMelds);
    bool checkFuriten(const vector<Tile>& waitingTiles, const bool* isDiscarded);

private:
    
};

#endif