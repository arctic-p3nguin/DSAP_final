#include "ScoreCounter.hpp"
#include "Tile.hpp"
#include "ShantenCounter.hpp"
#include <vector>
#include <algorithm>
#include <map>

using namespace std;


const vector<int> ScoreCounter::countTiles(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count(TILE_COUNT, 0); 

    for (const Tile& tile : concealed) {
        int index = tile.getType() * 9 + tile.getValue();
        if (index >= 0 && index < TILE_COUNT) {
            count[index]++;
        }
    }

    for (const Meld& meld : melds) {
        for (const Tile& tile : meld.tiles) {
            int index = tile.getType() * 9 + tile.getValue();
            if (index >= 0 && index < TILE_COUNT) {
                count[index]++;
            }
        }
    }
    return count;
}

int ScoreCounter::checkTanyao(const vector<int>& count) {
    if (count[0] > 0 || count[8] > 0 || count[9] > 0 || count[17] > 0 || 
        count[18] > 0 || count[26] > 0) return 0;
    for (int i = 27; i < 34; ++i) {
        if (count[i] > 0) return 0;
    }
    return 1;
}

int ScoreCounter::checkPinfu(const ParsedStructure& structData, bool hasMelds, int fieldWind, int seatWind) {
    if (hasMelds) return 0; 
    if (structData.sequences.size() != 4) return 0;

    int pairIdx = structData.pair;
    if (pairIdx == 27 + fieldWind || pairIdx == 27 + seatWind || pairIdx >= 31) return 0;

    return 1; 
}

int ScoreCounter::checkIipeiko(const ParsedStructure& structData, bool hasMelds) {
    if (hasMelds) return 0;
    map<int, int> seqCount;
    for (int startIdx : structData.sequences) {
        seqCount[startIdx]++;
    }
    int iipeikoCount = 0;
    for (auto const& [startIdx, cnt] : seqCount) {
        if (cnt >= 2) iipeikoCount++;
    }
    return (iipeikoCount == 1) ? 1 : 0;
}

int ScoreCounter::checkYakuFan(const vector<int>& count, int fieldWind) {
    int idx = 27 + fieldWind;
    return (count[idx] >= 3) ? 1 : 0;
}

int ScoreCounter::checkYakuhaiJia(const vector<int>& count, int selfWind) {
    int idx = 27 + selfWind;
    return (count[idx] >= 3) ? 1 : 0;
}

int ScoreCounter::checkYakuhaiSangen(const vector<int>& count) {
    int fan = 0;
    if (count[31] >= 3) fan++; 
    if (count[32] >= 3) fan++; 
    if (count[33] >= 3) fan++; 
    return fan;
}

int ScoreCounter::checkSanshokuDoujun(const ParsedStructure& structData, bool hasMelds) {
    for (int v = 0; v < 7; ++v) {
        bool hasMan = false, hasSou = false, hasPin = false;
        for (int startIdx : structData.sequences) {
            if (startIdx == v) hasMan = true;
            if (startIdx == 9 + v) hasSou = true;
            if (startIdx == 18 + v) hasPin = true;
        }
        if (hasMan && hasSou && hasPin) return hasMelds ? 1 : 2;
    }
    return 0;
}

int ScoreCounter::checkIttsu(const ParsedStructure& structData, bool hasMelds) {
    int offsets[] = {0, 9, 18};
    for (int offset : offsets) {
        bool has123 = false, has456 = false, has789 = false;
        for (int startIdx : structData.sequences) {
            if (startIdx == offset) has123 = true;
            if (startIdx == offset + 3) has456 = true;
            if (startIdx == offset + 6) has789 = true;
        }
        if (has123 && has456 && has789) return hasMelds ? 1 : 2;
    }
    return 0;
}

int ScoreCounter::checkChanta(const ParsedStructure& structData, bool hasMelds) {
    bool hasHonor = false;
    
    // 1. 雀頭檢查
    int pType = structData.pair / 9;
    int pVal = structData.pair % 9;
    if (pType == 3) hasHonor = true; // 字牌
    else if (pVal != 0 && pVal != 8) return 0; // 數牌但不是 1, 9

    // 2. 所有刻子檢查
    for (int t : structData.triplets) {
        int tType = t / 9;
        int tVal = t % 9;
        if (tType == 3) hasHonor = true;
        else if (tVal != 0 && tVal != 8) return 0;
    }

    // 3. 所有順子檢查
    for (int s : structData.sequences) {
        int sVal = s % 9;
        if (sVal != 0 && sVal != 6) return 0; // 必須是 123 或 789
    }

    if (!hasHonor) return 0; // 如果全無字牌，那是純全帶（由 checkJunchan 處理）

    return hasMelds ? 1 : 2;
}

int ScoreCounter::checkToitoi(const vector<int>& count) {
    int triplets = 0;
    for (int i = 0; i < TILE_COUNT; ++i) {
        if (count[i] >= 3) triplets++;
    }
    return (triplets == 4) ? 2 : 0;
}

int ScoreCounter::checkSanankou(const ParsedStructure& structData, const vector<Meld>& melds, bool isTsumo) {
    // 1. 先計算這組牌型結構中，總共有幾組刻子
    int ankouCount = structData.triplets.size();

    // 2. 扣掉副露中的明刻（碰、大明槓）
    for (const auto& m : melds) {
        if (m.type == MeldType::Pon || m.type == MeldType::Minkan) {
            ankouCount--;
        }
    }

    // 3. 【核心修正】：處理榮和（非自摸）時的雙碰聽牌盲點
    // 如果玩家沒有副露（門清），且結構拆解出 3 組刻子，但他是「榮和」胡牌：
    // 代表他一定是雙碰聽牌（例如手裡有 22 筒和 33 條，別人打 2 筒讓你榮和）
    // 這時候胡到的那一組 222 筒會被視為「明刻」，因此實質暗刻數要減 1（變成 2 組，三暗刻不成立）
    // 註：如果他手裡有 4 組刻子（聽單騎四暗刻），即使榮和一組變明刻，剩下的 3 組依然是暗刻，此時三暗刻依然成立！
    if (!isTsumo && ankouCount == 3) {
        ankouCount--;
    }

    return (ankouCount >= 3) ? 2 : 0;
}

int ScoreCounter::checkSanshokuDoukou(const vector<int>& count) {
    for (int v = 0; v < 9; ++v) {
        if (count[v] >= 3 && count[9 + v] >= 3 && count[18 + v] >= 3) {
            return 2;
        }
    }
    return 0;
}

int ScoreCounter::checkSankantsu(const vector<Meld>& melds) {
    int kantsu = 0;
    for (const auto& m : melds) {
        if (m.type == MeldType::Ankan || m.type == MeldType::Minkan) kantsu++;
    }
    return (kantsu == 3) ? 2 : 0;
}

int ScoreCounter::checkHonroutou(const vector<int>& count) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 1; j < 8; ++j) {
            if (count[i * 9 + j] > 0) return 0;
        }
    }
    return 2;
}

int ScoreCounter::checkChitoitsu(const vector<int>& count) {
    int pairs = 0;
    for (int i = 0; i < TILE_COUNT; ++i) {
        if (count[i] == 2) pairs++;
        if (count[i] == 4) return 0; 
    }
    return (pairs == 7) ? 2 : 0;
}

int ScoreCounter::checkRyanpeiko(const ParsedStructure& structData, bool hasMelds) {
    if (hasMelds) return 0;
    map<int, int> seqCount;
    for (int startIdx : structData.sequences) {
        seqCount[startIdx]++;
    }
    int pairsOfSeq = 0;
    for (auto const& [startIdx, cnt] : seqCount) {
        if (cnt >= 2) pairsOfSeq++;
        if (cnt == 4) pairsOfSeq += 2; 
    }
    return (pairsOfSeq == 2) ? 3 : 0;
}

int ScoreCounter::checkJunchan(const ParsedStructure& structData, bool hasMelds) {
    // 1. 檢查雀頭（將眼）：必須是數牌，且必須是 1 或 9 (pVal == 0 或 8)
    if (structData.pair >= 27) return 0; // 含有字牌，直接淘汰！
    int pVal = structData.pair % 9;
    if (pVal != 0 && pVal != 8) return 0; // 不是 1 或 9，淘汰！

    // 2. 檢查所有刻子：必須是數牌，且必須是 1 或 9 的刻子
    for (int t : structData.triplets) {
        if (t >= 27) return 0; // 含有字牌刻子，淘汰！
        if (t % 9 != 0 && t % 9 != 8) return 0; // 不是 1 或 9 的刻子，淘汰！
    }

    // 3. 檢查所有順子：必須存在順子，且所有順子都必須是 123 (s%9 == 0) 或 789 (s%9 == 6)
    if (structData.sequences.empty()) return 0; // 純全必須包含順子（否則會變成清老頭）
    for (int s : structData.sequences) {
        if (s % 9 != 0 && s % 9 != 6) return 0; // 只要有一組是 234、345 等中張順子，立刻淘汰！
    }

    // 4. 通過所有嚴格考驗，判定成立！門清 3 番，副露（食下） 2 番
    return hasMelds ? 2 : 3;
}

int ScoreCounter::checkHonitsu(const vector<int>& count, bool hasMelds) {
    int man = 0, sou = 0, pin = 0, zhi = 0;
    for (int i = 0; i < 9; ++i) {
        man += count[i]; sou += count[9 + i]; pin += count[18 + i];
    }
    for (int i = 27; i < TILE_COUNT; ++i) zhi += count[i];
    if (zhi == 0) return 0; 

    int suits = (man > 0) + (sou > 0) + (pin > 0);
    return (suits == 1) ? (hasMelds ? 2 : 3) : 0;
}

int ScoreCounter::checkChinitsu(const vector<int>& count, bool hasMelds) {
    int man = 0, sou = 0, pin = 0, zhi = 0;
    for (int i = 0; i < 9; ++i) {
        man += count[i]; sou += count[9 + i]; pin += count[18 + i];
    }
    for (int i = 27; i < TILE_COUNT; ++i) zhi += count[i];
    if (zhi > 0) return 0; 

    int suits = (man > 0) + (sou > 0) + (pin > 0);
    return (suits == 1) ? (hasMelds ? 5 : 6) : 0;
}

bool ScoreCounter::checkKokushiMusou(const vector<Tile>& concealed, const vector<Meld>& melds) {
    if (!melds.empty()) return false;
    vector<int> count = countTiles(concealed, melds);
    int kind = 0; 
    int totalKokushiTiles = 0;
    bool hasPair = false;
    int indices[] = {0,8,9,17,18,26,27,28,29,30,31,32,33};
    
    for (int idx : indices) {
        if (count[idx] >= 1) kind++;
        if (count[idx] >= 2) hasPair = true;
        totalKokushiTiles += count[idx];
    }
    // 確保 13 種都有，且這 13 種牌的加總剛好是手牌的 14 張（斷絕雜牌混入）
    return (kind == 13 && hasPair && totalKokushiTiles == 14);
}

bool ScoreCounter::checkSuuankou(const vector<Tile>& concealed, const vector<Meld>& melds) {
    if (!melds.empty()) return false; 
    vector<int> count = countTiles(concealed, melds);
    int triplets = 0;
    for (int i = 0; i < TILE_COUNT; ++i) { if (count[i] >= 3) triplets++; }
    return (triplets == 4);
}

bool ScoreCounter::checkDaisangen(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count = countTiles(concealed, melds);
    return (count[31] >= 3 && count[32] >= 3 && count[33] >= 3);
}

bool ScoreCounter::checkShousuushuu(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count = countTiles(concealed, melds);
    int triplets = 0, pair = 0;
    for (int i = 27; i <= 30; ++i) {
        if (count[i] >= 3) triplets++; else if (count[i] == 2) pair++;
    }
    return (triplets == 3 && pair == 1);
}

bool ScoreCounter::checkDaisuushuu(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count = countTiles(concealed, melds);
    int triplets = 0;
    for (int i = 27; i <= 30; ++i) { if (count[i] >= 3) triplets++; }
    return (triplets == 4);
}

bool ScoreCounter::checkTsuuiisou(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count = countTiles(concealed, melds);
    for (int i = 0; i < 27; ++i) { if (count[i] > 0) return false; }
    return true;
}

bool ScoreCounter::checkRyuuiisou(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count = countTiles(concealed, melds);
    int validIndices[] = {10, 11, 12, 14, 16, 32};
    int totalGreen = 0;
    for (int idx : validIndices) totalGreen += count[idx];
    int totalTiles = 0;
    for (int c : count) totalTiles += c;
    return (totalGreen == totalTiles);
}

bool ScoreCounter::checkChinroutou(const vector<Tile>& concealed, const vector<Meld>& melds) {
    vector<int> count = countTiles(concealed, melds);
    int validIndices[] = {0, 8, 9, 17, 18, 26};
    int totalOld = 0;
    for (int idx : validIndices) totalOld += count[idx];
    int totalTiles = 0;
    for (int c : count) totalTiles += c;
    return (totalOld == totalTiles && checkToitoi(count) > 0);
}

bool ScoreCounter::checkSuukantsu(const vector<Meld>& melds) {
    int kantsu = 0;
    for (const auto& m : melds) {
        if (m.type == MeldType::Ankan || m.type == MeldType::Minkan) kantsu++;
    }
    return (kantsu == 4);
}

bool ScoreCounter::checkChuurenPoutou(const vector<Tile>& concealed, const vector<Meld>& melds) {
    if (!melds.empty()) return false;
    vector<int> count = countTiles(concealed, melds);
    int suit = -1;
    if (count[0] > 0) suit = 0;
    else if (count[9] > 0) suit = 1;
    else if (count[18] > 0) suit = 2;
    else return false;

    int offset = suit * 9;
    if (count[offset] < 3 || count[offset + 8] < 3) return false;
    for (int i = 1; i <= 7; ++i) { if (count[offset + i] < 1) return false; }
    return true;
}

vector<string> ScoreResult::getYakuNames() const {
    static const map<YakuType, string> names = {
        {YakuType::Riichi, "Riichi"}, {YakuType::Ippatsu, "Ippatsu"}, {YakuType::MenzenTsumo, "Menzen Tsumo"},
        {YakuType::Pinfu, "Pinfu"}, {YakuType::Iipeiko, "Iipeiko"}, {YakuType::Tanyao, "Tanyao"},
        {YakuType::YakuhaiFan, "Value Tiles (Field Wind)"}, {YakuType::YakuhaiJia, "Value Tiles (Seat Wind)"}, {YakuType::YakuhaiSangen, "Value Tiles (Dragons)"},
        {YakuType::Chankan, "Chankan"}, {YakuType::Rinshan, "Rinshan Kaihou"}, {YakuType::Haitei, "Haitei Raoyue"}, {YakuType::Houtei, "Houtei Yuiyu"},
        {YakuType::DoubleRiichi, "Double Riichi"}, {YakuType::SanshokuDoujun, "Sanshoku Doujun"}, {YakuType::Ittsu, "Ittsu"},
        {YakuType::Chanta, "Chanta"}, {YakuType::Toitoi, "Toitoi"}, {YakuType::Sanankou, "Sanankou"},
        {YakuType::SanshokuDoukou, "Sanshoku Doukou"}, {YakuType::Sankantsu, "Sankantsu"}, {YakuType::Honroutou, "Honroutou"},
        {YakuType::Chitoitsu, "Chitoitsu"}, {YakuType::Ryanpeiko, "Ryanpeiko"}, {YakuType::Junchan, "Junchan"},
        {YakuType::Honitsu, "Honitsu"}, {YakuType::Chinitsu, "Chinitsu"},
        {YakuType::KokushiMusou, "Kokushi Musou"}, {YakuType::Suuankou, "Suuankou"}, {YakuType::Daisangen, "Daisangen"},
        {YakuType::Shousuushuu, "Shousuushuu"}, {YakuType::Daisuushuu, "Daisuushuu"}, {YakuType::Tsuuiisou, "Tsuuiisou"},
        {YakuType::Ryuuiisou, "Ryuuiisou"}, {YakuType::Chinroutou, "Chinroutou"}, {YakuType::Suukantsu, "Suukantsu"},
        {YakuType::ChuurenPoutou, "Chuuren Poutou"}, {YakuType::Tenhou, "Tenhou"}, {YakuType::Chihou, "Chihou"},
        {YakuType::Dora, "Dora"}
    };
    vector<string> res;
    for (auto y : yakuList) {
        if (names.count(y)) res.push_back(names.at(y));
    }
    return res;
}

ScoreResult ScoreCounter::calculateBestScore(const vector<Tile>& concealed,
                                             const vector<Meld>& melds,
                                             bool isRiichi, bool isIppatsu, bool isTsumo,
                                             bool isChankan, bool isRinshan, bool isHaitei, bool isHoutei,
                                             bool isDoubleRiichi, bool isTenhou, bool isChihou, bool isNagashiMangan,
                                             int fieldWind, int seatWind, bool isDealer,
                                             const DoraSystem& doraSys) {
    ShantenCounter sc;
    vector<ParsedStructure> allStructures;

    sc.calculateAllStructures(concealed, melds, allStructures);

    ScoreResult bestResult;
    bestResult.totalScore = -1;

    for (const auto& structure : allStructures) {
        ScoreResult currentRes = calculateForSpecificStructure(
            structure, concealed, melds,
            isRiichi, isIppatsu, isTsumo,
            isChankan, isRinshan, isHaitei, isHoutei,
            isDoubleRiichi, isTenhou, isChihou, isNagashiMangan,
            fieldWind, seatWind, isDealer, doraSys
        );

        if (currentRes.totalScore > bestResult.totalScore) {
            bestResult = currentRes;
        }
    }

    ParsedStructure fallbackStructure;
    if (sc.calculate(concealed, melds, fallbackStructure) == -1) {
        ScoreResult fallbackRes = calculateForSpecificStructure(
            fallbackStructure, concealed, melds,
            isRiichi, isIppatsu, isTsumo,
            isChankan, isRinshan, isHaitei, isHoutei,
            isDoubleRiichi, isTenhou, isChihou, isNagashiMangan,
            fieldWind, seatWind, isDealer, doraSys
        );
        if (fallbackRes.totalScore > bestResult.totalScore) {
            bestResult = fallbackRes;
        }
    }

    if (bestResult.totalScore < 0) {
        ScoreResult emptyRes;
        return emptyRes;
    }

    return bestResult;
}

ScoreResult ScoreCounter::calculateForSpecificStructure(
                                    const ParsedStructure& targetStructure,
                                    const vector<Tile>& concealed, 
                                    const vector<Meld>& melds,
                                    bool isRiichi, bool isIppatsu, bool isTsumo,
                                    bool isChankan, bool isRinshan, bool isHaitei, bool isHoutei,
                                    bool isDoubleRiichi, bool isTenhou, bool isChihou, bool isNagashiMangan,
                                    int fieldWind, int seatWind, bool isDealer,
                                    const DoraSystem& doraSys) {
    ScoreResult result;

    result.han = 0;
    result.fu = 0;

    vector<int> count = countTiles(concealed, melds);
    bool hasMelds = !melds.empty();

    if (isTenhou) { result.yakuList.push_back(YakuType::Tenhou); result.han = 13; return result; }
    if (isChihou) { result.yakuList.push_back(YakuType::Chihou); result.han = 13; return result; }

    bool isYakuman = false;
    if (targetStructure.pattern == HandPattern::Kokushi && checkKokushiMusou(concealed, melds)) { 
        result.yakuList.push_back(YakuType::KokushiMusou); isYakuman = true; 
    }
    if (targetStructure.pattern == HandPattern::Normal) {
        if (checkSuuankou(concealed, melds)) { result.yakuList.push_back(YakuType::Suuankou); isYakuman = true; }
        if (checkDaisangen(concealed, melds)) { result.yakuList.push_back(YakuType::Daisangen); isYakuman = true; }
        if (checkDaisuushuu(concealed, melds)) { result.yakuList.push_back(YakuType::Daisuushuu); isYakuman = true; }
        else if (checkShousuushuu(concealed, melds)) { result.yakuList.push_back(YakuType::Shousuushuu); isYakuman = true; }
        if (checkTsuuiisou(concealed, melds)) { result.yakuList.push_back(YakuType::Tsuuiisou); isYakuman = true; }
        if (checkRyuuiisou(concealed, melds)) { result.yakuList.push_back(YakuType::Ryuuiisou); isYakuman = true; }
        if (checkChinroutou(concealed, melds)) { result.yakuList.push_back(YakuType::Chinroutou); isYakuman = true; }
        if (checkSuukantsu(melds)) { result.yakuList.push_back(YakuType::Suukantsu); isYakuman = true; }
        if (checkChuurenPoutou(concealed, melds)) { result.yakuList.push_back(YakuType::ChuurenPoutou); isYakuman = true; }
    }
    if (isYakuman) { result.han = 13; return result; }

    if (isDoubleRiichi) { result.yakuList.push_back(YakuType::DoubleRiichi); result.han += 2; }
    else if (isRiichi) { result.yakuList.push_back(YakuType::Riichi); result.han += 1; }
    if ((isRiichi || isDoubleRiichi) && isIppatsu) { result.yakuList.push_back(YakuType::Ippatsu); result.han += 1; }
    if (isTsumo && !hasMelds) { result.yakuList.push_back(YakuType::MenzenTsumo); result.han += 1; }
    if (isChankan) { result.yakuList.push_back(YakuType::Chankan); result.han += 1; }
    if (isRinshan) { result.yakuList.push_back(YakuType::Rinshan); result.han += 1; }
    if (isTsumo && isHaitei) { result.yakuList.push_back(YakuType::Haitei); result.han += 1; }
    if (!isTsumo && isHoutei) { result.yakuList.push_back(YakuType::Houtei); result.han += 1; }

    int tanyao = checkTanyao(count);
    if (tanyao > 0) { result.yakuList.push_back(YakuType::Tanyao); result.han += tanyao; }
    int yakuFan = checkYakuFan(count, fieldWind);
    if (yakuFan > 0) { result.yakuList.push_back(YakuType::YakuhaiFan); result.han += yakuFan; }
    int yakuJia = checkYakuhaiJia(count, seatWind);
    if (yakuJia > 0) { result.yakuList.push_back(YakuType::YakuhaiJia); result.han += yakuJia; }
    int yakuSangen = checkYakuhaiSangen(count);
    if (yakuSangen > 0) { result.yakuList.push_back(YakuType::YakuhaiSangen); result.han += yakuSangen; }

    int toitoi = checkToitoi(count);
    if (toitoi > 0) { result.yakuList.push_back(YakuType::Toitoi); result.han += toitoi; }
    int sanshokuDoukou = checkSanshokuDoukou(count);
    if (sanshokuDoukou > 0) { result.yakuList.push_back(YakuType::SanshokuDoukou); result.han += sanshokuDoukou; }
    int sankantsu = checkSankantsu(melds);
    if (sankantsu > 0) { result.yakuList.push_back(YakuType::Sankantsu); result.han += sankantsu; }
    int honroutou = checkHonroutou(count);
    if (honroutou > 0) { result.yakuList.push_back(YakuType::Honroutou); result.han += honroutou; }
    int honitsu = checkHonitsu(count, hasMelds);
    if (honitsu > 0) { result.yakuList.push_back(YakuType::Honitsu); result.han += honitsu; }
    int chinitsu = checkChinitsu(count, hasMelds);
    if (chinitsu > 0) { result.yakuList.push_back(YakuType::Chinitsu); result.han += chinitsu; }

    if (targetStructure.pattern == HandPattern::Normal) {
        int pinfu = checkPinfu(targetStructure, hasMelds, fieldWind, seatWind);
        if (pinfu > 0) { result.yakuList.push_back(YakuType::Pinfu); result.han += pinfu; }

        int ryanpeiko = checkRyanpeiko(targetStructure, hasMelds);
        if (ryanpeiko > 0) {
            result.yakuList.push_back(YakuType::Ryanpeiko); result.han += ryanpeiko;
        } else {
            int iipeiko = checkIipeiko(targetStructure, hasMelds);
            if (iipeiko > 0) { result.yakuList.push_back(YakuType::Iipeiko); result.han += iipeiko; }
        }

        int sanshokuDoujun = checkSanshokuDoujun(targetStructure, hasMelds);
        if (sanshokuDoujun > 0) { result.yakuList.push_back(YakuType::SanshokuDoujun); result.han += sanshokuDoujun; }

        int ittsu = checkIttsu(targetStructure, hasMelds);
        if (ittsu > 0) { result.yakuList.push_back(YakuType::Ittsu); result.han += ittsu; }

        int chanta = checkChanta(targetStructure, hasMelds);
        if (chanta > 0) { result.yakuList.push_back(YakuType::Chanta); result.han += chanta; }

        int sanankou = checkSanankou(targetStructure, melds, isTsumo);
        if (sanankou > 0) { result.yakuList.push_back(YakuType::Sanankou); result.han += sanankou; }

        int junchan = checkJunchan(targetStructure, hasMelds);
        if (junchan > 0) { result.yakuList.push_back(YakuType::Junchan); result.han += junchan; }
    } 
    else if (targetStructure.pattern == HandPattern::Chitoitsu) {
        int chitoitsu = checkChitoitsu(count);
        if (chitoitsu > 0) { result.yakuList.push_back(YakuType::Chitoitsu); result.han += chitoitsu; }
    }

    bool hasValidYaku = !result.yakuList.empty();

    if (!hasValidYaku) {
        // 沒有起胡役，屬於「無役」，直接判定為不合法胡牌！
        result.han = 0; 
        result.totalScore = 0;
        return result; 
    }

    // 2. 走到這邊，代表「有合法起胡役」了！此時才可以開始疊加寶牌番數
    int doraCount = 0;

    // 取得目前所有生效的寶牌指示牌清單
    vector<Tile> activeDoras = doraSys.getActiveIndicators();
    vector<Tile> activeUraDoras = doraSys.getActiveUraIndicators();

    // 遍歷手牌中的門前清牌 (concealed)
    for (const Tile& tile : concealed) {
        // 檢查表寶牌
        for (const Tile& ind : activeDoras) {
            if (doraSys.isActualDoraTile(tile, ind)) doraCount++;
        }
        // 如果玩家有立直，才檢查裏寶牌
        if (isRiichi) {
            for (const Tile& ind : activeUraDoras) {
                if (doraSys.isActualDoraTile(tile, ind)) doraCount++;
            }
        }
    }

    // 遍歷副露中的牌 (melds)[cite: 5]
    for (const Meld& meld : melds) {
        for (const Tile& tile : meld.tiles) {
            // 檢查表寶牌（副露一樣有表寶牌）
            for (const Tile& ind : activeDoras) {
                if (doraSys.isActualDoraTile(tile, ind)) doraCount++;
            }
            // 檢查裏寶牌
            if (isRiichi) {
                for (const Tile& ind : activeUraDoras) {
                    if (doraSys.isActualDoraTile(tile, ind)) doraCount++;
                }
            }
        }
    }

    // 3. 如果有中寶牌，將番數加入總番數，增加 YakuType::Dora
    if (doraCount > 0) {
        result.han += doraCount;
        result.yakuList.push_back(YakuType::Dora); // 把註解拿掉，晚點來加這個 Enum
    }


    // 替換原本的 fu 計算區塊：
    if (result.han >= 13) {
        result.fu = 0; 
    } else {
        bool isChitoitsuYaku = false;
        bool isPinfuYaku = false;
        for (const auto& y : result.yakuList) {
            if (y == YakuType::Chitoitsu) isChitoitsuYaku = true;
            if (y == YakuType::Pinfu) isPinfuYaku = true;
        }

        if (isChitoitsuYaku) {
            result.fu = 25; 
        } else if (isPinfuYaku) {
            result.fu = isTsumo ? 20 : 30; 
        } else {
            int baseFu = 20; // 底符
            
            // A. 門前清榮和加成
            if (!hasMelds && !isTsumo) baseFu += 10; 
            // B. 自摸加成
            if (isTsumo) baseFu += 2;                
            
            // C. 加上雀頭的符數 (三元牌、場風、自風 +2 符，連風牌計 +2 或 +4 視規則)
            int pIdx = targetStructure.pair;
            if (pIdx >= 31 || pIdx == 27 + fieldWind || pIdx == 27 + seatWind) {
                baseFu += 2; 
            }

            // D. 加上刻子的符數
            // 註：這裡做一個簡單的明暗判定。如果副露中有對應刻子，視為明刻。
            for (int t : targetStructure.triplets) {
                bool isMinKou = false; // 是否為明刻
                for (const auto& m : melds) {
                    if ((m.type == MeldType::Pon || m.type == MeldType::Minkan) && 
                        (m.tiles[0].getType() * 9 + m.tiles[0].getValue() == t)) {
                        isMinKou = true;
                        break;
                    }
                }
                
                int tVal = t % 9;
                bool isYaochuu = (t >= 27 || tVal == 0 || tVal == 8); // 是否為么九牌
                
                if (isMinKou) {
                    baseFu += isYaochuu ? 4 : 2;  // 明刻：中張2符/么九4符
                } else {
                    baseFu += isYaochuu ? 8 : 4;  // 暗刻：中張4符/么九8符
                }
            }

            // E. 加上槓子的符數
            for (const auto& m : melds) {
                if (m.type == MeldType::Ankan || m.type == MeldType::Minkan) {
                    int tIdx = m.tiles[0].getType() * 9 + m.tiles[0].getValue();
                    bool isYaochuu = (tIdx >= 27 || tIdx % 9 == 0 || tIdx % 9 == 8);
                    if (m.type == MeldType::Ankan) {
                        baseFu += isYaochuu ? 32 : 16;
                    } else {
                        baseFu += isYaochuu ? 16 : 8;
                    }
                }
            }

            // 無條件進位到十位數
            result.fu = ((baseFu + 9) / 10) * 10; 
            if (result.fu < 30) result.fu = 30; 
        }
    }

    int han = result.han;
    int fu = result.fu;
    int baseScore = 0;

    if (han >= 13) { baseScore = 8000; }
    else if (han >= 11) { baseScore = 6000; }
    else if (han >= 8) { baseScore = 4000; }
    else if (han >= 6) { baseScore = 3000; }
    else if (han == 5) { baseScore = 2000; }
    else {
        baseScore = fu * (1 << (han + 2));
        if (baseScore >= 1920 || (han == 4 && fu >= 30) || (han == 3 && fu >= 60)) {
            baseScore = 2000; 
        }
    }

    if (isTsumo) {
        if (isDealer) {
            result.koPay = ceilToHundred(baseScore * 2);
            result.oyaPay = 0;
            result.totalScore = result.koPay * 3;
        } else {
            result.oyaPay = ceilToHundred(baseScore * 2);
            result.koPay = ceilToHundred(baseScore);
            result.totalScore = result.oyaPay + (result.koPay * 2);
        }
    } else {
        if (isDealer) { result.totalScore = ceilToHundred(baseScore * 6); }
        else { result.totalScore = ceilToHundred(baseScore * 4); }
        result.oyaPay = 0;
        result.koPay = 0;
    }

    return result;
}