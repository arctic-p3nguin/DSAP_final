#include "ShantenCounter.hpp"
#include <algorithm>

using namespace std;

// 檢查三種胡法的向聽然後再取他們裡面最小的
int ShantenCounter::calculate(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds, ParsedStructure& outBestStructure) {
    int count[TILE_COUNT] = {0};
    int meldsCnt = exposedMelds.size(); 

    for(Tile tile: concealedTiles) {
        int index = tile.getType() * 9 + tile.getValue();
        count[index]++;
    }

    int sh1 = countNormal(count, meldsCnt, outBestStructure);
    int sh2 = (meldsCnt == 0) ? countSevenPairs(count) : 8;
    int sh3 = (meldsCnt == 0) ? countThirteenOneNine(count) : 8;

    return min(sh1, min(sh2, sh3));
}

int ShantenCounter::calculateAllStructures(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds, vector<ParsedStructure>& outStructures) {
    int count[TILE_COUNT] = {0};
    int meldsCnt = exposedMelds.size();

    for(Tile tile : concealedTiles) {
        int index = tile.getType() * 9 + tile.getValue();
        count[index]++;
    }

    if(meldsCnt == 0) {
        if(countSevenPairs(count) == -1) {
            ParsedStructure spStruct;
            spStruct.pattern = HandPattern::Chitoitsu;
            spStruct.pair = -1;
            outStructures.push_back(spStruct);
        }
        if(countThirteenOneNine(count) == -1) {
            ParsedStructure kmStruct;
            kmStruct.pattern = HandPattern::Kokushi;
            kmStruct.pair = -1;
            outStructures.push_back(kmStruct);
        }
    }

    int maxScore = 0;
    ParsedStructure currentPath;

    for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] >= 2) {
            count[i] -= 2;
            currentPath = ParsedStructure();
            currentPath.pair = i;
            currentPath.pattern = HandPattern::Normal;
            shantenDfsGetAll(0, meldsCnt, 0, count, maxScore, currentPath, outStructures);
            count[i] += 2;
        }
    }

    return 8 - maxScore;
}

int ShantenCounter::countNormal(int count[], int meldsCnt, ParsedStructure& outBestStructure) {
    int maxScore = 0;
    int result = 8;
    ParsedStructure currentBest;

    for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] >= 2) {
            count[i] -= 2;
            
            ParsedStructure currentPath;
            currentPath.pattern = HandPattern::Normal;
            currentPath.pair = i;

            shantenDfs(0, true, meldsCnt, 0, count, maxScore, currentPath, currentBest);
            
            count[i] += 2;

            int shanten = 8 - maxScore - 1;
            if (shanten < result) {
                result = shanten;
                outBestStructure = currentBest;
            }
        }
    }

    maxScore = 0;
    ParsedStructure currentPath;
    currentPath.pattern = HandPattern::Normal;
    currentPath.pair = -1;

    shantenDfs(0, false, meldsCnt, 0, count, maxScore, currentPath, currentBest);
    
    int shanten = 8 - maxScore;
    if(shanten < result) {
        result = shanten;
        outBestStructure = currentBest;
    }

    return result;
}

void ShantenCounter::shantenDfs(int i, bool hasPair, int melds, int protos, int count[], int& maxScore, ParsedStructure& currentPath, ParsedStructure& bestStructure) {
    if(melds+protos == 4 || i >= TILE_COUNT) {
        int score = melds * 2 + protos;
        if(score > maxScore) {
            maxScore = score;
            bestStructure = currentPath; 
        }
        return;
    }

    if(melds*2 + protos + (4-melds-protos)*2 <= maxScore) {
        return;
    }

    // 刻子 (面子)
    if(count[i] >= 3) {
        count[i] -= 3;
        currentPath.triplets.push_back(i);
        shantenDfs(i, hasPair, melds + 1, protos, count, maxScore, currentPath, bestStructure);
        currentPath.triplets.pop_back();
        count[i] += 3;
    }

    // 順子 (面子)
    if(i < 27 && (i % 9) <= 6) {
        if (count[i] >= 1 && count[i+1] >= 1 && count[i+2] >= 1) {
            count[i]--; count[i+1]--; count[i+2]--;
            currentPath.sequences.push_back(i);
            shantenDfs(i, hasPair, melds + 1, protos, count, maxScore, currentPath, bestStructure);
            currentPath.sequences.pop_back();
            count[i]++; count[i+1]++; count[i+2]++;
        }
    }

    // 對子 (搭子)
    if(count[i] >= 2) {
        count[i] -= 2;
        shantenDfs(i + 1, hasPair, melds, protos + 1, count, maxScore, currentPath, bestStructure);
        count[i] += 2;
    }

    // 嵌張 / 兩面 (搭子)
    if(i < 27 && (i % 9) <= 7) {
        if(count[i] >= 1 && count[i+1] >= 1) {
            count[i]--; count[i+1]--;
            shantenDfs(i + 1, hasPair, melds, protos + 1, count, maxScore, currentPath, bestStructure);
            count[i]++; count[i+1]++;
        }
        if((i % 9) <= 6) {
            if(count[i] >= 1 && count[i+2] >= 1) {
                count[i]--; count[i+2]--;
                shantenDfs(i + 1, hasPair, melds, protos + 1, count, maxScore, currentPath, bestStructure);
                count[i]++; count[i+2]++;
            }
        }
    }

    // 孤張處理
    int backup = count[i];
    count[i] = 0;
    shantenDfs(i + 1, hasPair, melds, protos, count, maxScore, currentPath, bestStructure);
    count[i] = backup;
}

void ShantenCounter::shantenDfsGetAll(int i, int melds, int protos, int count[], int& maxScore, ParsedStructure& currentPath, vector<ParsedStructure>& outStructures) {
    if(melds+protos > 4) return;

    if(i >= TILE_COUNT) {
        if (melds == 4 && currentPath.pair != -1) {
            outStructures.push_back(currentPath);
        }
        return;
    }

    if(count[i] == 0) {
        shantenDfsGetAll(i+1, melds, protos, count, maxScore, currentPath, outStructures);
        return;
    }

    if(count[i] >= 3) {
        count[i] -= 3;
        currentPath.triplets.push_back(i);
        shantenDfsGetAll(i, melds+1, protos, count, maxScore, currentPath, outStructures);
        currentPath.triplets.pop_back();
        count[i] += 3;
    }

    if(i < 27 && (i % 9 <= 6)) {
        if(count[i] >= 1 && count[i+1] >= 1 && count[i+2] >= 1) {
            count[i]--; count[i+1]--; count[i+2]--;
            currentPath.sequences.push_back(i);
            shantenDfsGetAll(i, melds + 1, protos, count, maxScore, currentPath, outStructures);
            currentPath.sequences.pop_back();
            count[i]++; count[i+1]++; count[i+2]++;
        }
    }

    int backup = count[i];
    count[i] = 0;
    shantenDfsGetAll(i + 1, melds, protos, count, maxScore, currentPath, outStructures);
    count[i] = backup;
}

int ShantenCounter::countSevenPairs(int count[]) {
    int pair = 0, kind = 0, left;
    for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] >= 1) kind++;
        if(count[i] >= 2) pair++;
    }
    left = 6 - pair + max(0, 7-kind);
    return left; 
}

int ShantenCounter::countThirteenOneNine(int count[]) {
    int kind = 0, left;
    bool hasPair;

    hasPair = count[0]>=2 || count[8]>=2 || count[9]>=2 || count[17]>=2 || count[18]>=2 || count[26]>=2 || count[27]>=2 || count[28]>=2 || count[29]>=2 || count[30]>=2 || count[31]>=2 || count[32]>=2 || count[33]>=2;

    kind += (count[0]>=1 ? 1 : 0);
    kind += (count[8]>=1 ? 1 : 0);
    kind += (count[9]>=1 ? 1 : 0);
    kind += (count[17]>=1 ? 1 : 0);
    kind += (count[18]>=1 ? 1 : 0);
    kind += (count[26]>=1 ? 1 : 0);
    kind += (count[27]>=1 ? 1 : 0);
    kind += (count[28]>=1 ? 1 : 0);
    kind += (count[29]>=1 ? 1 : 0);
    kind += (count[30]>=1 ? 1 : 0);
    kind += (count[31]>=1 ? 1 : 0);
    kind += (count[32]>=1 ? 1 : 0);
    kind += (count[33]>=1 ? 1 : 0);
    
    left = 13 - kind - (hasPair ? 1 : 0);

    return left;
}
