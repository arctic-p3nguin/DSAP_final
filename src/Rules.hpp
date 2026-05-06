#ifndef RULES_HPP
#define RULES_HPP

#include <vector>
#include "Tile.hpp"

using namespace std;

class Rules {
public:
    bool isWin(const vector<Tile>& concealedTiles);

private:
    // 一般的和
    bool checkWinning(int count[]);
    // 特殊和
    bool checkSevenPairs(int count[]);
    bool checkThirteenOneNine(int count[]);
};

#endif