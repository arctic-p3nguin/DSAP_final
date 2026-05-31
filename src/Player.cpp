#include "Player.hpp"
#include "ScoreCounter.hpp"
#include <climits>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <cstring>

using namespace std;

Player::Player(string n, int s)
    : name(n), seat(s), isDealer(false), isRiichi(false), score(25000), temporalFuriten(false) {
    for(int i = 0; i < 34; i++) {
        remainingTileCounts[i] = 4;
    }
}

void Player::resetRoundState() {
    hand.clear();
    river.clear();
    melds.clear();

    isRiichi = false;
    temporalFuriten = false;

    for(int i = 0; i < TILE_COUNT; i++) {
        isDiscarded[i] = false;
    }

    for(int i = 0; i < 34; i++) {
        remainingTileCounts[i] = 4;
    }
}

void Player::drawTile(const Tile& tile) {
    temporalFuriten = false;
    hand.addTile(tile);
}

Tile Player::discardTile(const Tile& tile) {
    Tile discarded = hand.discardTile(tile.getType(), tile.getValue(), tile.getId());

    if(discarded.getType() < 0) {
        const auto& concealed = hand.getConcealed();
        if (!concealed.empty()) {
            const Tile& fallback = concealed.back();
            discarded = hand.discardTile(fallback.getType(), fallback.getValue(), fallback.getId());
        }
    }

    if(discarded.getType() >= 0 && discarded.getValue() >= 0) {
        river.push_back(discarded);
        int index = discarded.getType() * 9 + discarded.getValue();
        if(index >= 0 && index < TILE_COUNT) {
            isDiscarded[index] = true;
        }
    }

    return discarded;
}

vector<int> Player::getActualRemainingTiles() {
    vector<int> actualRemaining(34);
    for(int i = 0; i < 34; i++) {
        actualRemaining[i] = remainingTileCounts[i];
    }

    for(const Tile& tile : hand.getConcealed()) {
        int idx = tile.getType() * 9 + tile.getValue();
        actualRemaining[idx]--;
    }

    for(int i = 0; i < 34; i++) {
        if (actualRemaining[i] < 0) actualRemaining[i] = 0;
    }

    return actualRemaining;
}

void Player::decreaseValidTile(int type, int value, int count) {
    int idx = type * 9 + value;
    if(idx >= 0 && idx < 34) {
        remainingTileCounts[idx] -= count;
        if(remainingTileCounts[idx] < 0) {
            remainingTileCounts[idx] = 0;
        }
    }
}

void Player::performChi(const vector<Tile>& combo, const Tile& discardedTile) {
    for (const auto& t : combo) {
        hand.discardTile(t.getType(), t.getValue(), t.getId());
    }
    vector<Tile> meldContents = combo;
    meldContents.push_back(discardedTile);
    hand.addMeld({MeldType::Chi, meldContents});
}

void Player::performPon(const Tile& discardedTile) {
    for (int i = 0; i < 2; i++) {
        hand.discardTile(discardedTile.getType(), discardedTile.getValue(), -1);
    }
    vector<Tile> meldContents(3, discardedTile);
    hand.addMeld({MeldType::Pon, meldContents});
}

void Player::performKan(const Tile& discardedTile, MeldType kanType) {
    vector<Tile> meldContents;
    int removeCount = (kanType == MeldType::Ankan) ? 4 : 3;
    for (int i = 0; i < removeCount; i++) {
        hand.discardTile(discardedTile.getType(), discardedTile.getValue(), -1);
    }
    meldContents.assign(4, discardedTile);
    hand.addMeld({kanType, meldContents});
}

void Player::displayStatus() const {
    cout << "[" << name << " Seated " << seat << "]" << (isDealer ? " *Dealer*" : "") << "\n";
    cout << "River Size: " << river.size() << "\n";
}

const bool* Player::getIsDiscarded() const {
    return isDiscarded;
}

int Player::getWaitingTilesCount() {
    Rules rules;
    vector<Tile> waitingTiles = rules.getWaitingTiles(hand.getConcealed(), hand.getMelds());

    int totalRemaining = 0;
    for(const auto& wTile : waitingTiles) {
        int idx = wTile.getType() * 9 + wTile.getValue();
        if (idx >= 0 && idx < 34) {
            totalRemaining += remainingTileCounts[idx];
        }
    }
    return totalRemaining;
}

bool Player::hasOtherYakuThanRiichi() {
    if (!doraSys) return false;

    Rules rules;
    ScoreCounter scoreCounter;

    vector<Tile> waitingTiles = rules.getWaitingTiles(hand.getConcealed(), hand.getMelds());
    if (waitingTiles.empty()) return false;

    for (const Tile& w : waitingTiles) {
        vector<Tile> simConcealed = hand.getConcealed();
        simConcealed.push_back(w);

        ScoreResult res = scoreCounter.calculateBestScore(
            simConcealed, hand.getMelds(),
            false, false, false,
            false, false, false, false,
            false, false, false, false,
            0, seat, isDealer,
            *doraSys
        );

        if (res.han > 0) {
            return true;
        }
    }
    return false;
}

int Player::calculateEffectiveTilesCount(const vector<Tile>& concealed, const vector<Meld>& melds) {
    int count = 0;
    ShantenCounter scTemp;
    ParsedStructure dummy;
    int baseShanten = scTemp.calculate(concealed, melds, dummy);
    if(baseShanten == -1) return 0;

    for(int type = 0; type < 4; type++) {
        int maxVal = (type == 3) ? 7 : 9;
        for(int value = 0; value < maxVal; value++) {
            Tile t(type, value, -1, false);
            vector<Tile> nextConcealed = concealed;
            nextConcealed.push_back(t);
            int newShanten = scTemp.calculate(nextConcealed, melds, dummy);
            if(newShanten < baseShanten) {
                int idx = type * 9 + value;
                count += remainingTileCounts[idx];
            }
        }
    }
    return count;
}


Tile RandomPlayer::decideDiscard(const vector<Player*>& allPlayers) {
    const vector<Tile>& currentTiles = hand.getConcealed();
    
    int randomIndex = rand() % currentTiles.size();
    return currentTiles[randomIndex];
}

Tile ShantenPlayer::decideDiscard(const vector<Player*>& allPlayers) {
    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();
    ParsedStructure dummyStructure;

    int bestShanten = 8;
    Tile toDiscard = curTiles.back(); 
    vector<Tile> tempTiles = curTiles;
    // 枚舉丟手上的牌 然後看哪一張丟出去的向聽數會最低就選他
    for(int i = 0; i < curTiles.size(); i++) {
        Tile tempDiscard = tempTiles[i];
        tempTiles.erase(tempTiles.begin() + i);

        int shanten = sc.calculate(tempTiles, curMelds, dummyStructure);
        if(shanten < bestShanten) {
            bestShanten = shanten;
            toDiscard = tempDiscard;
        }

        tempTiles.insert(tempTiles.begin() + i, tempDiscard);

        if(bestShanten <= 0) break;
    }
    return toDiscard;
}


// 計算目標役種距離、寶牌加成跟無役懲罰
int SpeedPlayer::evaluateHandPotential(const vector<Tile>& concealed, const vector<Meld>& melds, const DoraSystem& doraSys) {
    int yakuBonus = 0;
    bool hasConfirmedYaku = false;

    int count[34] = {0};
    for(const auto& t : concealed) count[t.getType() * 9 + t.getValue()]++;
    for(const auto& m : melds) {
        for (const auto& t : m.tiles) count[t.getType() * 9 + t.getValue()]++;
    }

    // 判斷自風、場風、三元
    int fieldWind = 0; // 東風場
    int seatWind = this->seat; 
    int yakuIndices[] = {31, 32, 33, 27 + fieldWind, 27 + seatWind}; // 白發中、場風、自風
    
    for(int idx : yakuIndices) {
        if(count[idx] >= 3) {
            yakuBonus += 500;
            hasConfirmedYaku = true;
        } 
        else if(count[idx] == 2) {
            yakuBonus += 150;
        }
    }

    // 判斷斷幺九距離
    int oneNineCount = 0;
    int oneNineIndices[] = {0, 8, 9, 17, 18, 26}; // 19萬、19索、19筒
    for(int idx : oneNineIndices) oneNineCount += count[idx];
    for(int i = 27; i < 34; i++) oneNineCount += count[i]; // 字牌

    if(oneNineCount == 0) {
        yakuBonus += 400; 
        hasConfirmedYaku = true;
    } 
    else if (oneNineCount == 1) {
        yakuBonus += 250; 
    } 
    else if (oneNineCount == 2) {
        yakuBonus += 100; 
    }

    // 判斷混一色距離
    int man = 0, sou = 0, pin = 0, zi = 0;
    for (int i = 0; i < 9; i++)  man += count[i];
    for (int i = 9; i < 18; i++) sou += count[i]; 
    for (int i = 18; i < 27; i++) pin += count[i];
    for (int i = 27; i < 34; i++) zi += count[i];

    int maxSuit = max({man, sou, pin});
    int totalHonitsuTiles = maxSuit + zi; 

    if(totalHonitsuTiles >= 9) {
        yakuBonus += (totalHonitsuTiles - 8) * 100;
        if(totalHonitsuTiles >= 11) { // 距離2以內
            hasConfirmedYaku = true; 
        }
    }

    // 計算寶牌加成
    int doraCount = 0;
    vector<Tile> activeDoras = doraSys.getActiveIndicators();
    auto checkDora = [&](const Tile& t) {
        for(const Tile& ind : activeDoras) {
            if (doraSys.isActualDoraTile(t, ind)) doraCount++; 
        }
    };
    for(const Tile& t : concealed) checkDora(t);
    for(const Meld& m : melds) {
        for (const Tile& t : m.tiles) checkDora(t);
    }
    int doraBonus = doraCount * 100;

    // 無役懲罰
    if(!melds.empty() && !hasConfirmedYaku) {
        ParsedStructure dummy;
        int currentShanten = sc.calculate(concealed, melds, dummy);

        if(currentShanten <= 0) {
            return -9999; // 聽牌了還無役，這絕對不行，維持重罰
        } 
        else if(currentShanten == 1) {
            return -1500; // 一向聽無役，警告！要開始修正役種了
        } 
        else {
            return -300;  // 二向聽以上無役，完全沒關係，先吃碰搶速度再說
        }
    }

    return yakuBonus + doraBonus;
}

Tile SpeedPlayer::decideDiscard(const vector<Player*>& allPlayers) {
    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();
    
    int bestScore = -1e9+7;
    Tile toDiscard = curTiles.back(); 
    vector<Tile> tempTiles = curTiles;


    // 枚舉切掉手上的哪一張牌
    for(size_t i = 0; i < curTiles.size(); i++) {
        Tile tempDiscard = tempTiles[i];
        tempTiles.erase(tempTiles.begin() + i);

        ParsedStructure dummy;
        int shanten = sc.calculate(tempTiles, curMelds, dummy);
        int effectiveCount = calculateEffectiveTilesCount(tempTiles, curMelds);
        int potential = evaluateHandPotential(tempTiles, curMelds, *doraSys);

        int currentScore = ((8-shanten) * 1000) + (effectiveCount * 10) + potential;

        if(currentScore > bestScore) {
            bestScore = currentScore;
            toDiscard = tempDiscard;
        }

        // 還原
        tempTiles.insert(tempTiles.begin() + i, tempDiscard);
    }

    return toDiscard;
}

CallType SpeedPlayer::decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canMinkan) {
    if(getIsRiichi()) return CallType::None;

    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();
    
    ParsedStructure dummy;
    int currentShanten = sc.calculate(curTiles, curMelds, dummy);
    int currentEffective = calculateEffectiveTilesCount(curTiles, curMelds);
    int currentPotential = evaluateHandPotential(curTiles, curMelds, *doraSys);

    int baseNoCallScore = ((8 - currentShanten) * 1000) + (currentEffective * 10) + currentPotential;
    if(curMelds.empty()) {
        baseNoCallScore += 100; 
    }

    int bestActionScore = baseNoCallScore;
    CallType bestCall = CallType::None;

    // 明槓
    if(canMinkan) {
        vector<Tile> tempConcealed = curTiles;
        int removed = 0;
        for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ) {
            if(it->isSameKind(discardedTile) && removed < 3) {
                it = tempConcealed.erase(it);
                removed++;
            } 
            else {
                it++;
            }
        }
        vector<Meld> tempMelds = curMelds;
        tempMelds.push_back({MeldType::Minkan, vector<Tile>(4, discardedTile)});
        
        int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
        int newEffective = calculateEffectiveTilesCount(tempConcealed, tempMelds);
        int newPotential = evaluateHandPotential(tempConcealed, tempMelds, *doraSys);

        if(newPotential != -9999) {
            int minkanScore = ((8 - newShanten) * 1000) + (newEffective * 10) + newPotential;
            if(minkanScore > bestActionScore) {
                bestActionScore = minkanScore;
                bestCall = CallType::Minkan;
            }
        }
    }

    // 碰
    if(canPon) {
        vector<Tile> tempConcealed = curTiles;
        int removed = 0;
        for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ) {
            if(it->isSameKind(discardedTile) && removed < 2) {
                it = tempConcealed.erase(it);
                removed++;
            } 
            else {
                it++;
            }
        }
        vector<Meld> tempMelds = curMelds;
        tempMelds.push_back({MeldType::Pon, vector<Tile>(3, discardedTile)});
        
        int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
        int newEffective = calculateEffectiveTilesCount(tempConcealed, tempMelds);
        int newPotential = evaluateHandPotential(tempConcealed, tempMelds, *doraSys);

        if(newPotential != -9999) {
            int ponScore = ((8 - newShanten) * 1000) + (newEffective * 10) + newPotential;
            if(ponScore > bestActionScore) {
                bestActionScore = ponScore;
                bestCall = CallType::Pon;
            }
        }
    }

    // 吃
    if(canChi) {
        vector<vector<Tile>> chiCombos = rules.getChiCombinations(curTiles, discardedTile);
        for (const auto& combo : chiCombos) {
            vector<Tile> tempConcealed = curTiles;
            for(const auto& comboTile : combo) {
                for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ++it) {
                    if(it->getType() == comboTile.getType() && it->getValue() == comboTile.getValue() && it->getId() == comboTile.getId()) {
                        tempConcealed.erase(it);
                        break;
                    }
                }
            }
            
            vector<Tile> meldContents = combo;
            meldContents.push_back(discardedTile);
            vector<Meld> tempMelds = curMelds;
            tempMelds.push_back({MeldType::Chi, meldContents});

            int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
            int newEffective = calculateEffectiveTilesCount(tempConcealed, tempMelds);
            int newPotential = evaluateHandPotential(tempConcealed, tempMelds, *doraSys);

            if(newPotential != -9999) {
                int chiScore = ((8 - newShanten) * 1000) + (newEffective * 10) + newPotential;
                if(chiScore > bestActionScore) {
                    bestActionScore = chiScore;
                    bestCall = CallType::Chi;
                }
            }
        }
    }

    return bestCall;
}

// defense player實作
int DefensePlayer::evaluateHandPotential(const vector<Tile>& concealed, const vector<Meld>& melds, const DoraSystem& doraSys) {
    int yakuBonus = 0;
    bool hasConfirmedYaku = false;

    int count[34] = {0};
    for(const auto& t : concealed) count[t.getType() * 9 + t.getValue()]++;
    for(const auto& m : melds) {
        for (const auto& t : m.tiles) count[t.getType() * 9 + t.getValue()]++;
    }

    // 判斷自風、場風、三元
    int fieldWind = 0; // 東風場
    int seatWind = this->seat; 
    int yakuIndices[] = {31, 32, 33, 27 + fieldWind, 27 + seatWind}; // 白發中、場風、自風
    
    for(int idx : yakuIndices) {
        if(count[idx] >= 3) {
            yakuBonus += 500;
            hasConfirmedYaku = true;
        } 
        else if(count[idx] == 2) {
            yakuBonus += 150;
        }
    }

    // 判斷斷幺九距離
    int oneNineCount = 0;
    int oneNineIndices[] = {0, 8, 9, 17, 18, 26}; // 19萬、19索、19筒
    for(int idx : oneNineIndices) oneNineCount += count[idx];
    for(int i = 27; i < 34; i++) oneNineCount += count[i]; // 字牌

    if(oneNineCount == 0) {
        yakuBonus += 400; 
        hasConfirmedYaku = true;
    } 
    else if (oneNineCount == 1) {
        yakuBonus += 250; 
    } 
    else if (oneNineCount == 2) {
        yakuBonus += 100; 
    }

    // 判斷混一色距離
    int man = 0, sou = 0, pin = 0, zi = 0;
    for (int i = 0; i < 9; i++)  man += count[i];
    for (int i = 9; i < 18; i++) sou += count[i]; 
    for (int i = 18; i < 27; i++) pin += count[i];
    for (int i = 27; i < 34; i++) zi += count[i];

    int maxSuit = max({man, sou, pin});
    int totalHonitsuTiles = maxSuit + zi; 

    if(totalHonitsuTiles >= 9) {
        yakuBonus += (totalHonitsuTiles - 8) * 100;
        if(totalHonitsuTiles >= 11) { // 距離2以內
            hasConfirmedYaku = true; 
        }
    }

    // 計算寶牌加成
    int doraCount = 0;
    vector<Tile> activeDoras = doraSys.getActiveIndicators();
    auto checkDora = [&](const Tile& t) {
        for(const Tile& ind : activeDoras) {
            if (doraSys.isActualDoraTile(t, ind)) doraCount++; 
        }
    };
    for(const Tile& t : concealed) checkDora(t);
    for(const Meld& m : melds) {
        for (const Tile& t : m.tiles) checkDora(t);
    }
    int doraBonus = doraCount * 100;

    // 無役懲罰
    if(!melds.empty() && !hasConfirmedYaku) return -9999; 

    return yakuBonus + doraBonus;
}

Tile DefensePlayer::decideDiscard(const vector<Player*>& allPlayers) {
    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();

    ParsedStructure dummy;
    int currentSelfShanten = sc.calculate(curTiles, curMelds, dummy);

    // 找出所有有聽牌可能的敵人
    vector<pair<const Player*, double>> enemies;
    double highestThreatOnField = 0.0; 

    for (const Player* enemy : allPlayers) {
        if (enemy == this) continue; 

        double currentEnemyThreat = 0.0;
        if(enemy->getIsRiichi()) {
            currentEnemyThreat = 1.0; 
        }
        else {
            int meldCount = enemy->getHand().getMelds().size();
            if(meldCount == 2) currentEnemyThreat = 0.4;      // 副露威脅
            else if(meldCount == 3) currentEnemyThreat = 0.7;
            else if(meldCount == 4) currentEnemyThreat = 1.0;
        }
        if(currentEnemyThreat > highestThreatOnField) {
            highestThreatOnField = currentEnemyThreat;
        }
        enemies.emplace_back(enemy, currentEnemyThreat);
    }

    // 枚舉切牌 計算評分
    double bestScore = -1e9;
    Tile toDiscard = curTiles.back();
    vector<Tile> tempTiles = curTiles;

    for(size_t i = 0; i < curTiles.size(); i++) {
        Tile tempDiscard = tempTiles[i];
        tempTiles.erase(tempTiles.begin() + i);

        // 計算基礎速度分數
        ParsedStructure dummyStructure;
        int shanten = sc.calculate(tempTiles, curMelds, dummyStructure);
        int effectiveCount = calculateEffectiveTilesCount(tempTiles, curMelds);
        int potential = evaluateHandPotential(tempTiles, curMelds, *doraSys);
        
        double speedScore = ((8 - shanten) * 1000) + (effectiveCount * 10) + potential;

        // 計算針對該威脅敵人的安全度加分
        double maxSafetyScore = 0.0;
        bool isFullyDropping = false; // 是否棄和

        if (highestThreatOnField >= 1.0 && currentSelfShanten > 0) {
            isFullyDropping = true;
        }

        for(auto& [enemy, maxThreat] : enemies) {
            if (maxThreat <= 0.0) continue;

            double safetyScore = 15.0; 
            int type = tempDiscard.getType();
            int val = tempDiscard.getValue();
            int idx = type * 9 + val;

            // 現物
            if(enemy->getIsDiscarded()[idx] || this->getIsDiscarded()[idx]) {
                safetyScore = 100.0; 
            }
            // 字牌
            else if(type == 3) { 
                int visibleCount = 4 - remainingTileCounts[idx];
                if(visibleCount == 3) safetyScore = 95.0;      
                else if(visibleCount == 2) safetyScore = 75.0; 
                else if(visibleCount == 1) safetyScore = 45.0; 
                else safetyScore = 30.0;                        
            } 
             // 數牌
            else {
                bool isNoChance = false;
                if(val == 0 && (4-remainingTileCounts[type * 9 + 1] == 4)) isNoChance = true; 
                if(val == 8 && (4-remainingTileCounts[type * 9 + 7] == 4)) isNoChance = true; 
                
                if(isNoChance) {
                    safetyScore = 90.0;
                } 
                else {
                    // 筋牌
                    bool isSuji = false;
                    const bool* enemyDiscards = enemy->getIsDiscarded();

                    if(val == 0 && enemyDiscards[type * 9 + 3]) isSuji = true; 
                    else if(val == 8 && enemyDiscards[type * 9 + 5]) isSuji = true; 
                    else if(val == 1 && enemyDiscards[type * 9 + 4]) isSuji = true; 
                    else if(val == 7 && enemyDiscards[type * 9 + 4]) isSuji = true; 
                    else if(val == 3 && enemyDiscards[type * 9 + 0]) isSuji = true; 
                    else if(val == 5 && enemyDiscards[type * 9 + 8]) isSuji = true; 

                    if(isSuji) {
                        if(val == 0 || val == 8) safetyScore = 65.0; 
                        else safetyScore = 45.0; 
                    } 
                    else {
                        if(val >= 3 && val <= 5) safetyScore = 0.0; // 絕對危險中張
                        else safetyScore = 20.0;
                    }
                }
            }
            
            double defenseScale = 1.0; 

            if(maxThreat >= 1.0) { 
                if(currentSelfShanten == 0) {
                    defenseScale = 0.0; // 聽牌對攻
                } 
                else if(currentSelfShanten == 1) {
                    defenseScale = (potential >= 500) ? 0.5 : 4.0; 
                } 
                else {
                    defenseScale = 10.0; // 兩向聽以上，放大權重
                }
            } 

            double defenseWeight = 100.0 * defenseScale; 
            double defenseBonus = maxThreat * defenseWeight * safetyScore;
            if(defenseBonus > maxSafetyScore) {
                maxSafetyScore = defenseBonus;
            }
        } 

        double currentScore = isFullyDropping ? maxSafetyScore : (speedScore + maxSafetyScore);

        if(currentScore > bestScore) {
            bestScore = currentScore;
            toDiscard = tempDiscard;
        }

        tempTiles.insert(tempTiles.begin() + i, tempDiscard);
    }

    return toDiscard;
}

CallType DefensePlayer::decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canMinkan) {
    if (getIsRiichi()) return CallType::None;

    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();
    
    ParsedStructure dummy;
    int currentShanten = sc.calculate(curTiles, curMelds, dummy);
    int currentEffective = calculateEffectiveTilesCount(curTiles, curMelds);
    int currentPotential = evaluateHandPotential(curTiles, curMelds, *doraSys);

    int baseNoCallScore = ((8 - currentShanten) * 1000) + (currentEffective * 10) + currentPotential;
    
    // 門清留存權重
    if(curMelds.empty()) {
        baseNoCallScore += 1500; 
    }

    // 副露懲罰加重
    int meldPenalty = 0;
    size_t currentMeldCount = curMelds.size();
    
    if(currentMeldCount == 1) meldPenalty = 1500;   
    else if(currentMeldCount == 2) meldPenalty = 3000;  
    else if(currentMeldCount >= 3) meldPenalty = 6000;  

    int bestActionScore = baseNoCallScore;
    CallType bestCall = CallType::None;

    // 明槓
    if(canMinkan) {
        vector<Tile> tempConcealed = curTiles;
        int removed = 0;
        for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ) {
            if(it->isSameKind(discardedTile) && removed < 3) {
                it = tempConcealed.erase(it);
                removed++;
            } 
            else it++; 
        }
        vector<Meld> tempMelds = curMelds;
        tempMelds.push_back({MeldType::Minkan, vector<Tile>(4, discardedTile)});
        
        int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
        int newEffective = calculateEffectiveTilesCount(tempConcealed, tempMelds);
        int newPotential = evaluateHandPotential(tempConcealed, tempMelds, *doraSys);

        if(newPotential != -9999) {
            int minkanScore = ((8-newShanten) * 1000) + (newEffective * 10) + newPotential - meldPenalty;
            if(minkanScore > bestActionScore) {
                bestActionScore = minkanScore;
                bestCall = CallType::Minkan;
            }
        }
    }

    // 碰
    if(canPon) {
        vector<Tile> tempConcealed = curTiles;
        int removed = 0;
        for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ) {
            if(it->isSameKind(discardedTile) && removed < 2) {
                it = tempConcealed.erase(it);
                removed++;
            } 
            else it++;
        }
        vector<Meld> tempMelds = curMelds;
        tempMelds.push_back({MeldType::Pon, vector<Tile>(3, discardedTile)});
        
        int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
        int newEffective = calculateEffectiveTilesCount(tempConcealed, tempMelds);
        int newPotential = evaluateHandPotential(tempConcealed, tempMelds, *doraSys);

        if(newPotential != -9999) {
            int ponScore = ((8 - newShanten) * 1000) + (newEffective * 10) + newPotential - meldPenalty;
            if(ponScore > bestActionScore) {
                bestActionScore = ponScore;
                bestCall = CallType::Pon;
            }
        }
    }

    // 吃
    if(canChi) {
        vector<vector<Tile>> chiCombos = rules.getChiCombinations(curTiles, discardedTile);
        for(const auto& combo : chiCombos) {
            vector<Tile> tempConcealed = curTiles;
            for(const auto& comboTile : combo) {
                for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ++it) {
                    if(it->getType() == comboTile.getType() && it->getValue() == comboTile.getValue() && it->getId() == comboTile.getId()) {
                        tempConcealed.erase(it);
                        break;
                    }
                }
            }
            
            vector<Tile> meldContents = combo;
            meldContents.push_back(discardedTile);
            vector<Meld> tempMelds = curMelds;
            tempMelds.push_back({MeldType::Chi, meldContents});

            int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
            int newEffective = calculateEffectiveTilesCount(tempConcealed, tempMelds);
            int newPotential = evaluateHandPotential(tempConcealed, tempMelds, *doraSys);

            if(newPotential != -9999) {
                int chiScore = ((8-newShanten) * 1000) + (newEffective * 10) + newPotential - meldPenalty;
                if(chiScore > bestActionScore) {
                    bestActionScore = chiScore;
                    bestCall = CallType::Chi;
                }
            }
        }
    }

    return bestCall;
}

// HighScorePlayer 實作
int HighScorePlayer::evaluateHighScorePotential(const vector<Tile>& concealed, const vector<Meld>& melds, const DoraSystem& doraSys, int shanten) {
    bool hasMelds = !melds.empty();

    int count[34] = {0};
    for(const auto& t : concealed) count[t.getType()*9 + t.getValue()]++;
    for(const auto& m : melds) {
        for (const auto& t : m.tiles) count[t.getType()*9 + t.getValue()]++;
    }

    int totalTiles = 0;
    for (int i = 0; i < 34; i++) totalTiles += count[i];

    int man = 0, sou = 0, pin = 0, zi = 0;
    int pairCount = 0, tripletCount = 0;
    int terminalHonorCount = 0;
    int middleCount = 0;

    for(int i = 0; i < 34; i++) {
        int c = count[i];
        if (i < 9) man += c;
        else if (i < 18) sou += c;
        else if (i < 27) pin += c;
        else zi += c;

        if (c >= 2) pairCount++;
        if (c >= 3) tripletCount++;

        bool isYaochuu = (i >= 27 || i % 9 == 0 || i % 9 == 8);
        if (isYaochuu) terminalHonorCount += c;
        if (i < 27 && (i % 9 >= 1 && i % 9 <= 7)) middleCount += c;
    }

    int fieldWind = 0;
    int seatWind = this->seat;

    int yakuhaiTriplets = 0;
    int yakuhaiPairs = 0;
    int yakuIndices[] = {31, 32, 33, 27 + fieldWind, 27 + seatWind};
    for (int idx : yakuIndices) {
        if (idx < 0 || idx >= 34) continue;
        if (count[idx] >= 3) yakuhaiTriplets++;
        else if (count[idx] == 2) yakuhaiPairs++;
    }

    int doraCount = 0;
    vector<Tile> activeDoras = doraSys.getActiveIndicators();
    auto checkDora = [&](const Tile& t) {
        for(const Tile& ind : activeDoras) {
            if(doraSys.isActualDoraTile(t, ind)) doraCount++;
        }
    };
    for(const Tile& t : concealed) checkDora(t);
    for(const Meld& m : melds) {
        for(const Tile& t : m.tiles) checkDora(t);
    }

    int maxSuit = max({man, sou, pin});
    int dominantSuit = 0;
    if(maxSuit == man) dominantSuit = 0;
    else if(maxSuit == sou) dominantSuit = 1;
    else dominantSuit = 2;

    int suitTiles = maxSuit + zi;
    int otherSuitTiles = totalTiles - suitTiles;

    int yakuhaiScore = yakuhaiTriplets * 1700 + yakuhaiPairs * 500;

    int honitsuScore = 0;
    if(suitTiles >= 8) {
        if(zi > 0) {
            honitsuScore = 900 + (suitTiles - 8) * 280 + zi * 120 - otherSuitTiles * 120;
            if(maxSuit >= 9) honitsuScore += 900;
        } 
        else {
            honitsuScore = 1200 + (maxSuit - 8) * 420 - otherSuitTiles * 160;
            if(maxSuit >= 10) honitsuScore += 1200;
        }
        if(honitsuScore < 0) honitsuScore = 0;
    }

    int chinitsuScore = 0;
    if(zi == 0 && maxSuit >= 7) {
        chinitsuScore = 1500 + (maxSuit - 7) * 520 - otherSuitTiles * 180;
        if(maxSuit >= 10) chinitsuScore += 1800;
        if(chinitsuScore < 0) chinitsuScore = 0;
    }

    int toitoiScore = 0;
    if(tripletCount >= 1 || pairCount >= 3) {
        toitoiScore = tripletCount * 900 + pairCount * 220;
        if(tripletCount >= 2) toitoiScore += 700;
        if(tripletCount >= 3) toitoiScore += 1400;
        if(tripletCount >= 4) toitoiScore += 2600;
    }

    int chitoiScore = 0;
    if(!hasMelds) {
        chitoiScore = pairCount * 420;
        if(pairCount >= 4) chitoiScore += 800;
        if(pairCount >= 5) chitoiScore += 1300;
        if(pairCount >= 6) chitoiScore += 1900;
        if(pairCount >= 7) chitoiScore += 2800;
        if(tripletCount > 0) chitoiScore -= tripletCount * 200;
        if(chitoiScore < 0) chitoiScore = 0;
    }

    int terminalScore = 0;
    if(terminalHonorCount >= 7) {
        terminalScore = 700 + (terminalHonorCount - 7) * 150;
        if(middleCount <= 4) terminalScore += 600;
        if(zi == 0 && terminalHonorCount >= 8) terminalScore += 900;
    }

    int tanyaoScore = 0;
    if(zi == 0 && terminalHonorCount == 0) {
        tanyaoScore = 1100 + middleCount * 25;
    } 
    else if(zi == 0 && terminalHonorCount <= 2) {
        tanyaoScore = 500;
    }

    int suitRouteScore = max(honitsuScore, chinitsuScore);
    int pairRouteScore = hasMelds ? toitoiScore : max(toitoiScore, chitoiScore);
    int mainRouteScore = max({suitRouteScore, pairRouteScore, terminalScore, tanyaoScore});

    int synergyScore = 0;
    if(yakuhaiTriplets > 0 && suitRouteScore > 0) synergyScore += 900;
    if(yakuhaiTriplets > 0 && pairRouteScore > 0) synergyScore += 1200;
    if(suitRouteScore > 0 && pairRouteScore > 0) synergyScore += 900;
    if(terminalScore > 0 && yakuhaiTriplets > 0) synergyScore += 500;
    if(!hasMelds && pairCount >= 5) synergyScore += 400;

    int potentialScore = 0;
    potentialScore += yakuhaiScore;
    potentialScore += mainRouteScore;
    potentialScore += synergyScore;
    potentialScore += doraCount * 1200;

    if(!hasMelds) {
        potentialScore += 800;
        if(doraCount >= 2) potentialScore += 500;
    }

    if(hasMelds && yakuhaiScore == 0 && suitRouteScore == 0 && pairRouteScore == 0 && terminalScore == 0 && doraCount == 0) {
        return -9999;
    }

    double confidence = 1.0;
    if(shanten <= 0) confidence = 1.0;
    else if(shanten == 1) confidence = 0.96;
    else if(shanten == 2) confidence = 0.92;
    else if(shanten == 3) confidence = 0.88;
    else confidence = 0.84;

    return static_cast<int>(potentialScore * confidence);
}

Tile HighScorePlayer::decideDiscard(const vector<Player*>& allPlayers) {
    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();

    int bestScore = -1e9 + 7;
    Tile toDiscard = curTiles.back();
    vector<Tile> tempTiles = curTiles;

    for(size_t i = 0; i < curTiles.size(); i++) {
        Tile tempDiscard = tempTiles[i];
        tempTiles.erase(tempTiles.begin() + i);

        ParsedStructure dummyStructure;
        int shanten = sc.calculate(tempTiles, curMelds, dummyStructure);
        int potential = evaluateHighScorePotential(tempTiles, curMelds, *doraSys, shanten);

        int currentScore = ((8 - shanten) * 180) + potential;

        if(currentScore > bestScore) {
            bestScore = currentScore;
            toDiscard = tempDiscard;
        }

        tempTiles.insert(tempTiles.begin() + i, tempDiscard);
    }
    return toDiscard;
}

CallType HighScorePlayer::decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canMinkan) {
    if (getIsRiichi()) return CallType::None;

    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();

    ParsedStructure dummy;
    int currentShanten = sc.calculate(curTiles, curMelds, dummy);
    int currentPotential = evaluateHighScorePotential(curTiles, curMelds, *doraSys, currentShanten);

    int baseNoCallScore = ((8 - currentShanten) * 180) + currentPotential;
    if(curMelds.empty()) baseNoCallScore += 500;

    int bestActionScore = baseNoCallScore;
    CallType bestCall = CallType::None;

    int fieldWind = 0;
    int seatWind = seat;
    int discardIdx = discardedTile.getType() * 9 + discardedTile.getValue();
    bool isYakuhaiTile = (discardedTile.getType() == 3) &&
                         (discardedTile.getValue() >= 4 ||
                          discardedTile.getValue() == fieldWind ||
                          discardedTile.getValue() == seatWind);
    bool isTerminalTile = (discardedTile.getType() < 3) &&
                          (discardedTile.getValue() == 0 || discardedTile.getValue() == 8);

    int man = 0, sou = 0, pin = 0, zi = 0;
    for(const auto& t : curTiles) {
        if(t.getType() == 0) man++;
        else if(t.getType() == 1) sou++;
        else if(t.getType() == 2) pin++;
        else zi++;
    }

    int maxSuit = max({man, sou, pin});
    int dominantSuit = 0;
    if(maxSuit == man) dominantSuit = 0;
    else if(maxSuit == sou) dominantSuit = 1;
    else dominantSuit = 2;

    auto routeBonus = [&](const vector<Tile>& tempConcealed, const vector<Meld>& tempMelds, int newShanten, CallType callType) -> int {
        int newPotential = evaluateHighScorePotential(tempConcealed, tempMelds, *doraSys, newShanten);
        if (newPotential == -9999) return -999999;

        int score = ((8 - newShanten) * 180) + newPotential;

        if(callType == CallType::Pon) {
            score += 350;
            if(isYakuhaiTile) score += 1400;
            if(isTerminalTile) score += 250;
        } 
        else if(callType == CallType::Minkan) {
            score += 650;
            if(isYakuhaiTile) score += 1800;
            if(isTerminalTile) score += 300;
        } 
        else if(callType == CallType::Chi) {
            score += 50;
            if(discardedTile.getType() == dominantSuit && discardedTile.getType() < 3) score += 700;
            if(isTerminalTile) score += 150;
        }

        return score;
    };

    if(canMinkan) {
        vector<Tile> tempConcealed = curTiles;
        int removed = 0;
        for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ) {
            if(it->isSameKind(discardedTile) && removed < 3) {
                it = tempConcealed.erase(it);
                removed++;
            } 
            else {
                it++;
            }
        }
        vector<Meld> tempMelds = curMelds;
        tempMelds.push_back({MeldType::Minkan, vector<Tile>(4, discardedTile)});

        int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
        int minkanScore = routeBonus(tempConcealed, tempMelds, newShanten, CallType::Minkan);

        if(minkanScore > bestActionScore + 150) {
            bestActionScore = minkanScore;
            bestCall = CallType::Minkan;
        }
    }

    if(canPon) {
        vector<Tile> tempConcealed = curTiles;
        int removed = 0;
        for (auto it = tempConcealed.begin(); it != tempConcealed.end(); ) {
            if(it->isSameKind(discardedTile) && removed < 2) {
                it = tempConcealed.erase(it);
                removed++;
            } 
            else {
                it++;
            }
        }
        vector<Meld> tempMelds = curMelds;
        tempMelds.push_back({MeldType::Pon, vector<Tile>(3, discardedTile)});

        int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
        int ponScore = routeBonus(tempConcealed, tempMelds, newShanten, CallType::Pon);

        if(ponScore > bestActionScore + 120) {
            bestActionScore = ponScore;
            bestCall = CallType::Pon;
        }
    }

    if(canChi) {
        vector<vector<Tile>> chiCombos = rules.getChiCombinations(curTiles, discardedTile);
        for (const auto& combo : chiCombos) {
            vector<Tile> tempConcealed = curTiles;
            for(const auto& comboTile : combo) {
                for(auto it = tempConcealed.begin(); it != tempConcealed.end(); ++it) {
                    if(it->getType() == comboTile.getType() &&
                        it->getValue() == comboTile.getValue() &&
                        it->getId() == comboTile.getId()) {
                        tempConcealed.erase(it);
                        break;
                    }
                }
            }

            vector<Tile> meldContents = combo;
            meldContents.push_back(discardedTile);
            vector<Meld> tempMelds = curMelds;
            tempMelds.push_back({MeldType::Chi, meldContents});

            int newShanten = sc.calculate(tempConcealed, tempMelds, dummy);
            int chiScore = routeBonus(tempConcealed, tempMelds, newShanten, CallType::Chi);

            if(chiScore > bestActionScore + 250) {
                bestActionScore = chiScore;
                bestCall = CallType::Chi;
            }
        }
    }

    return bestCall;
}