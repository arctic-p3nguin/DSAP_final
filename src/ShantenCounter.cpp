#include "ShantenCounter.hpp"

using namespace std;

// 檢查三種胡法的向聽然後再取他們裡面最小的
int ShantenCounter::calculate(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds) {
    int count[TILE_COUNT] = {0};
    int meldsCnt = exposedMelds.size(); 

    for(Tile tile: concealedTiles) {
        int index = tile.getType() * 9 + tile.getValue();
        count[index]++;
    }

    int sh1 = countNormal(count, meldsCnt);
    int sh2 = (meldsCnt == 0) ? countSevenPairs(count) : 8;
    int sh3 = (meldsCnt == 0) ? countThirteenOneNine(count) : 8;

    return min(sh1, min(sh2, sh3));
}

//先拆掉雀頭 再處理剩下的牌(用dfs)
int ShantenCounter::countNormal(int count[], int meldsCnt) {
    int maxScore, result = 8;
        for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] >= 2){
            maxScore = 0;
            count[i] -= 2;
            shantenDfs(0, true, meldsCnt, 0, count, maxScore);
            result = min(result, 8 - maxScore/10*2 - maxScore%10 - 1);
            count[i] += 2;
        }
    }

    int maxScoreNoPair = 0;
    shantenDfs(0, false, meldsCnt, 0, count, maxScoreNoPair); // 沒有雀頭
    result = min(result, 8 - maxScoreNoPair/10*2 - maxScoreNoPair%10);

    return result;
}

void ShantenCounter::shantenDfs(int curIndex, bool hasPair, int melds, int protos, int count[], int& maxScore) {
    int i = curIndex;

    while(i < TILE_COUNT && count[i] == 0) i++;

    if(i >= TILE_COUNT) {
        int trueProtos = min(protos, 4-melds);
        maxScore = max(maxScore, melds*10 + trueProtos);
        return;
    }
    
    // 組刻子 
    if(count[i] >= 3) {
        count[i] -= 3;
        shantenDfs(i, hasPair, melds+1, protos, count, maxScore);
        count[i] += 3;
    }
    // 組順子
    if(i < TILE_COUNT-VALUE_COUNT_Z && i%VALUE_COUNT_N < 7) {
        if(count[i+1] >= 1 && count[i+2] >= 1) {
            count[i]--; 
            count[i+1]--; 
            count[i+2]--;
            shantenDfs(i, hasPair, melds+1, protos, count, maxScore);
            count[i]++; 
            count[i+1]++; 
            count[i+2]++;
        }
    }

    if(melds + protos < 4) {
        // 對子當搭子
        if(count[i] >= 2) {
            count[i] -= 2;
            shantenDfs(i, hasPair, melds, protos+1, count, maxScore);
            count[i] += 2;
        }
        // 邊張或崁張當搭子
        if(i < TILE_COUNT - VALUE_COUNT_Z) {
            // 邊
            if(i % VALUE_COUNT_N < 8 && count[i+1] >= 1) {
                count[i]--; 
                count[i+1]--;
                shantenDfs(i, hasPair, melds, protos+1, count, maxScore);
                count[i]++; 
                count[i+1]++;
            }
            // 崁
            if(i % VALUE_COUNT_N < 7 && count[i+2] >= 1) {
                count[i]--; 
                count[i+2]--;
                shantenDfs(i, hasPair, melds, protos+1, count, maxScore);
                count[i]++; 
                count[i+2]++;
            }
        }
    }

    // 孤張 沒辦法拿來湊的
    int backup = count[i];
    count[i] = 0;
    shantenDfs(i + 1, hasPair, melds, protos, count, maxScore);
    count[i] = backup;
}

int ShantenCounter::countSevenPairs(int count[]) {
    //七對子
    int pair = 0, kind = 0, left;
    for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] >= 1) kind++;
        if(count[i] >= 2) pair++;
    }

    left = 6 - pair + max(0, 7-kind);

    return left; 
}

int ShantenCounter::countThirteenOneNine(int count[]) {
    // 國士無雙
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