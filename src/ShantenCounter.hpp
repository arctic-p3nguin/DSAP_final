#ifndef SHANTEN_COUNTER_HPP
#define SHANTEN_COUNTER_HPP

#include <vector>
#include "Tile.hpp"
#include "Hand.hpp"

using namespace std;

// 定義胡牌的牌型分類
enum class HandPattern { 
    Normal,     // 常規面子型（4面子+1雀頭）
    Chitoitsu,  // 七對子
    Kokushi     // 國士無雙
};

struct ParsedStructure {
    HandPattern pattern = HandPattern::Normal; // 預設為常規面子
    int pair = -1;
    vector<int> triplets;
    vector<int> sequences;
};

class ShantenCounter {
public:
    // 保留你原本的 calculate（如果其他地方有用到）
    int calculate(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds, ParsedStructure& outBestStructure);

    // 全新：支援高點法，能抓出所有胡牌結構的接口
    int calculateAllStructures(const vector<Tile>& concealedTiles, 
                                const vector<Meld>& exposedMelds, 
                                vector<ParsedStructure>& outStructures);
    
                                

private:
    int countNormal(int count[], int meldsCnt, ParsedStructure& outBestStructure);
    void shantenDfs(int i, bool hasPair, int melds, int protos, int count[], int& maxScore, ParsedStructure& currentPath, ParsedStructure& bestStructure);
    int countSevenPairs(int count[]);
    int countThirteenOneNine(int count[]);


    // 全新：收集常規面子拆解的輔助 DFS
    void collectAllNormalStructures(int count[], int meldsCnt, vector<ParsedStructure>& outStructures);
    void shantenDfsGetAll(int i, int melds, int protos, int count[], int& maxScore, ParsedStructure& currentPath, vector<ParsedStructure>& outStructures);
};

#endif