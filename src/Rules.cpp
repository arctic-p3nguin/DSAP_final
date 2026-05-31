# include <vector>
# include "Rules.hpp"
# include "ShantenCounter.hpp"

using namespace std;


// 檢查是否有合法的吃牌組合，並回傳所有可能從手牌拿出來的2張牌組合
vector<vector<Tile>> Rules::getChiCombinations(const vector<Tile>& concealedTiles, const Tile& discardedTile) {
    vector<vector<Tile>> validCombos;
    
    // 字牌不能吃
    if (discardedTile.getType() == 3) return validCombos;

    int t = discardedTile.getType();
    int v = discardedTile.getValue();

    // 三種可能的組合
    int targets[3][2] = {
        {v-2, v-1},
        {v-1, v+1},
        {v+1, v+2}
    };

    for(int i = 0; i < 3; i++) {
        int v1 = targets[i][0];
        int v2 = targets[i][1];


        // 檢查手牌
        bool found1 = false, found2 = false;
        Tile match1(0, 0, 0, 0), match2(0, 0, 0, 0);

        for (const Tile& tile : concealedTiles) {
            if(tile.getType() == t) {
                if(!found1 && tile.getValue() == v1) {
                    match1 = tile;
                    found1 = true;
                } 
                else if(!found2 && tile.getValue() == v2) {
                    match2 = tile;
                    found2 = true;
                }
            }
        }

        // 如果手牌同時擁有這兩張牌就可以吃
        if(found1 && found2) {
            validCombos.push_back({match1, match2});
        }
    }

    return validCombos;
}

bool Rules::isWin(const vector<Tile>& concealedTiles, const vector<Meld>& exposedMelds) {
    ShantenCounter sc;
    ParsedStructure dummyStructure;
    int sh = sc.calculate(concealedTiles, exposedMelds, dummyStructure);
    if(sh == -1) return true;
    else return false;
}

vector<Tile> Rules::getWaitingTiles(vector<Tile> concealedTiles, const vector<Meld>& exposedMelds) {
    // 把所有牌都試過一次，然後看哪幾張牌能讓向聽數變成-1
    vector<Tile> waitingTiles;
    ShantenCounter sc;
    ParsedStructure dummyStructure;

    for(int type = 0; type < 4; type++) {
        for(int value = 0; value < 9; value++) {
            if(type == 3 && value > 6) break;

            Tile tile(type, value, -1, false);
            concealedTiles.push_back(tile);
            int newSh = sc.calculate(concealedTiles, exposedMelds, dummyStructure);
            if(newSh == -1) {
                waitingTiles.push_back(tile);
            }
            concealedTiles.pop_back();
        }
    }
    return waitingTiles;
}

// 檢查有沒有振聽(看河面)
bool Rules::checkFuriten(const vector<Tile>& waitingTiles, const bool* isDiscarded) {
    for(const Tile& w : waitingTiles) {
        int index = w.getType() * 9 + w.getValue();
        if (index >= 0 && index < 34 && isDiscarded[index]) {
            return true; 
        }
    }
    return false;
}
