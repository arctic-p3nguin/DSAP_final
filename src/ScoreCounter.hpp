#ifndef SCORE_COUNTER_HPP
#define SCORE_COUNTER_HPP

#include <vector>
#include <string>
#include "Tile.hpp"
#include "ShantenCounter.hpp"
//#include "GameManager.hpp"
#include "Player.hpp"

#ifndef TILE_COUNT_DEFINE
#define TILE_COUNT_DEFINE
#endif

using namespace std;

enum class YakuType {
    None, Riichi, Ippatsu, MenzenTsumo, Pinfu, Tanyao, Iipeiko,
    YakuhaiFan, YakuhaiJia, YakuhaiSangen, DoubleRiichi,
    Chankan, Rinshan, Haitei, Houtei, SanshokuDoujun, Ittsu, Chanta,
    Toitoi, Sanankou, SanshokuDoukou, Sankantsu, Honroutou, Chitoitsu,
    Ryanpeiko, Junchan, Honitsu, Chinitsu,
    Tenhou, Chihou, KokushiMusou, Suuankou, Daisangen, Shousuushuu,
    Daisuushuu, Tsuuiisou, Ryuuiisou, Chinroutou, Suukantsu, ChuurenPoutou,
    Dora
};

struct ScoreResult {
    int han = 0;
    int fu = 0;
    int totalScore = 0;
    int oyaPay = 0;
    int koPay = 0;
    vector<YakuType> yakuList;

    vector<string> getYakuNames() const;
};


class ScoreCounter {
public:
    ScoreResult calculateBestScore(const vector<Tile>& concealed, 
                                   const vector<Meld>& melds,
                                   bool isRiichi, bool isIppatsu, bool isTsumo,
                                   bool isChankan, bool isRinshan, bool isHaitei, bool isHoutei,
                                   bool isDoubleRiichi, bool isTenhou, bool isChihou, bool isNagashiMangan,
                                   int fieldWind, int seatWind, bool isDealer,
                                   const DoraSystem& doraSys);

    ScoreResult calculateForSpecificStructure(
                                   const ParsedStructure& targetStructure,
                                   const vector<Tile>& concealed, 
                                   const vector<Meld>& melds,
                                   bool isRiichi, bool isIppatsu, bool isTsumo,
                                   bool isChankan, bool isRinshan, bool isHaitei, bool isHoutei,
                                   bool isDoubleRiichi, bool isTenhou, bool isChihou, bool isNagashiMangan,
                                   int fieldWind, int seatWind, bool isDealer,
                                   const DoraSystem& doraSys);

private:
    int ceilToHundred(int score) {
        return ((score + 99) / 100) * 100;
    }

    const vector<int> countTiles(const vector<Tile>& concealed, const vector<Meld>& melds);
    int checkTanyao(const vector<int>& count);
    int checkPinfu(const ParsedStructure& structData, bool hasMelds, int fieldWind, int seatWind);
    int checkIipeiko(const ParsedStructure& structData, bool hasMelds);
    int checkYakuFan(const vector<int>& count, int fieldWind);
    int checkYakuhaiJia(const vector<int>& count, int selfWind);
    int checkYakuhaiSangen(const vector<int>& count);
    int checkSanshokuDoujun(const ParsedStructure& structData, bool hasMelds);
    int checkIttsu(const ParsedStructure& structData, bool hasMelds);
    int checkChanta(const ParsedStructure& structData, bool hasMelds);
    int checkToitoi(const vector<int>& count);
    int checkSanankou(const ParsedStructure& structData, const vector<Meld>& melds, bool isTsumo);
    int checkSanshokuDoukou(const vector<int>& count);
    int checkSankantsu(const vector<Meld>& melds);
    int checkHonroutou(const vector<int>& count);
    int checkChitoitsu(const vector<int>& count);
    int checkRyanpeiko(const ParsedStructure& structData, bool hasMelds);
    int checkJunchan(const ParsedStructure& structData, bool hasMelds);
    int checkHonitsu(const vector<int>& count, bool hasMelds);
    int checkChinitsu(const vector<int>& count, bool hasMelds);
    
    bool checkKokushiMusou(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkSuuankou(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkDaisangen(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkShousuushuu(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkDaisuushuu(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkTsuuiisou(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkRyuuiisou(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkChinroutou(const vector<Tile>& concealed, const vector<Meld>& melds);
    bool checkSuukantsu(const vector<Meld>& melds);
    bool checkChuurenPoutou(const vector<Tile>& concealed, const vector<Meld>& melds);
};

#endif