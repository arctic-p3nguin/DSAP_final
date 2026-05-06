#ifndef SHANTENCOUNTER_HPP
#define SHANTENCOUNTER_HPP

#include <vector>
#include <algorithm>
#include "Tile.hpp"
#include "Hand.hpp"

using namespace std;

class ShantenCounter {
public:
    // 傳入手牌，然後回傳最終向聽數 (最小值)
    int calculate(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds);

private:

    // 計算各種型態的向聽數
    int countNormal(int count[], int meldsCnt);      // 一般型
    int countSevenPairs(int count[]);  // 七對子
    int countThirteenOneNine(int count[]); // 國士無雙

    // dfs算湊刻子順子或搭子
    void shantenDfs(int curIndex, bool hasPair, int melds, int protos, int count[], int& maxScore);
};

#endif