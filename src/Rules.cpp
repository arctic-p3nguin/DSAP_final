# include <vector>
# include "Rules.hpp"

// 之後改 isWin判定可以改成向聽數=-1就好 
// prototype交完之後要新增役種判定和分數計算(用在進階邏輯)
using namespace std;

bool Rules::isWin(const vector<Tile>& concealedTiles) {
    int count[TILE_COUNT] = {0};
    for(Tile tile: concealedTiles) {
        int index = tile.getType() * 9 + tile.getValue();
        count[index]++;
    }

    bool result = false;
    // 先檢查七對子和國士
    result = checkSevenPairs(count);
    if(!result) result = checkThirteenOneNine(count);

    if(!result){
        for(int i = 0; i < TILE_COUNT; i++) {
            if(count[i] >= 2){
                count[i] -= 2;
                result = checkWinning(count);
                count[i] += 2;
            }
            if(result) break;
        }
    }
    return result;
}

bool Rules::checkWinning(int count[]) {
    int first = -1;
    for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] > 0) {
            first = i;
            break;
        }
    }

    // Base case
    if(first == -1) return true;

    // 刻子
    if(count[first] >= 3) {
        count[first] -= 3;
        if(checkWinning(count)) return true;
        count[first] += 3; 
    }

    // 順子
    if(first < 27 && (first % 9) <= 6) {
        if(count[first] > 0 && count[first+1] > 0 && count[first+2] > 0) {
            count[first]--;
            count[first+1]--;
            count[first+2]--;
            if(checkWinning(count)) return true;
            count[first]++; 
            count[first+1]++;
            count[first+2]++;
        }
    }

    return false;
}

bool Rules::checkSevenPairs(int count[]) {
    int cnt = 0;
    for(int i = 0; i < TILE_COUNT; i++) {
        if(count[i] != 0 && count[i] != 2){
            break;
        }
        if(count[i] == 2){
            cnt++;
        }
    }

    if(cnt == 7) return true;
    else return false;
}

bool Rules::checkThirteenOneNine(int count[]) {
    vector<int> temp(5, 0);
    temp[count[0]]++;
    temp[count[8]]++;
    temp[count[9]]++;
    temp[count[17]]++;
    temp[count[18]]++;
    temp[count[26]]++;
    temp[count[27]]++;
    temp[count[28]]++;
    temp[count[29]]++;
    temp[count[30]]++;
    temp[count[31]]++;
    temp[count[32]]++;
    temp[count[33]]++;
    if(temp[1] == 13 && temp[2] == 1) return true;
    else return false;
}