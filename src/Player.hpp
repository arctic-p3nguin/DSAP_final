#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <vector>
#include <string>
#include "ShantenCounter.hpp"
#include "Rules.hpp"

using namespace std;

enum class CallType { None, Chi, Pon, Ankan, Minkan, Kakan };

class ScoreCounter;

struct DoraSystem {
    vector<Tile> indicators;
    vector<Tile> uraIndicators;
    int activeCount = 1;

    vector<Tile> getActiveIndicators() const {
        return vector<Tile>(indicators.begin(), indicators.begin() + activeCount);
    }

    vector<Tile> getActiveUraIndicators() const {
        return vector<Tile>(uraIndicators.begin(), uraIndicators.begin() + activeCount);
    }

    bool isActualDoraTile(const Tile& handTile, const Tile& indicator) const {
        int targetValue = indicator.getValue() + 1;

        if (indicator.getType() == 3) {
            if (indicator.getValue() < 4) {
                if (targetValue > 3) targetValue = 0;
            } else {
                if (targetValue > 6) targetValue = 4;
            }
        } else {
            if (targetValue > 8) targetValue = 0;
        }
        return (handTile.getType() == indicator.getType() && handTile.getValue() == targetValue);
    }
};

class Player {
protected:
    string name;
    int seat;
    bool isDealer;
    Hand hand;
    vector<Tile> river;
    vector<Meld> melds;
    bool isRiichi;
    const DoraSystem* doraSys = nullptr;
    int score;
    bool temporalFuriten = false;

    bool isDiscarded[TILE_COUNT] = {false};
    int remainingTileCounts[TILE_COUNT];

public:
    Player(string n, int s);
    virtual ~Player() {}

    void setDealer(bool dealer) { isDealer = dealer; }
    bool getIsDealer() const { return isDealer; }
    void setIsRiichi(bool state) { isRiichi = state; }
    bool getIsRiichi() const { return isRiichi; }
    int getSeat() const { return seat; }

    int getScore() const { return score; }
    void setScore(int s) { score = s; }

    bool getTemporalFuriten() const { return temporalFuriten; }
    void setTemporalFuriten(bool furiten) { temporalFuriten = furiten; }

    const vector<Tile>& getRiver() const { return river; }
    const Hand& getHand() const { return hand; }

    void drawTile(const Tile& tile);
    Tile discardTile(const Tile& tile);
    void decreaseValidTile(int type, int value, int count);
    vector<int> getActualRemainingTiles();
    int calculateEffectiveTilesCount(const vector<Tile>& concealed, const vector<Meld>& melds);

    void setDoraSystem(const DoraSystem& ds) { doraSys = &ds; }

    virtual Tile decideDiscard(const vector<Player*>& allPlayers) = 0;
    virtual CallType decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canKan) {
        return CallType::None;
    }

    void performChi(const vector<Tile>& combo, const Tile& discardedTile);
    void performPon(const Tile& discardedTile);
    void performKan(const Tile& discardedTile, MeldType kanType);

    void displayStatus() const;
    const bool* getIsDiscarded() const;

    int getWaitingTilesCount();
    bool hasOtherYakuThanRiichi();

    void resetRoundState();
};

class RandomPlayer : public Player {
public:
    RandomPlayer(string n, int s) : Player(n, s) {}
    Tile decideDiscard(const vector<Player*>& allPlayers) override;
};

class ShantenPlayer : public Player {
private:
    ShantenCounter sc;

public:
    ShantenPlayer(string n, int s) : Player(n, s) {}
    Tile decideDiscard(const vector<Player*>& allPlayers) override;
};

class SpeedPlayer : public Player {
private:
    ShantenCounter sc;
    Rules rules;

public:
    SpeedPlayer(string n, int s) : Player(n, s) {}
    int evaluateHandPotential(const vector<Tile>& concealed, const vector<Meld>& melds, const DoraSystem& doraSys);
    virtual Tile decideDiscard(const vector<Player*>& allPlayers) override;
    virtual CallType decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canMinkan) override;
};

class DefensePlayer : public Player {
private:
    ShantenCounter sc;
    Rules rules;
    int evaluateHandPotential(const vector<Tile>& concealed, const vector<Meld>& melds, const DoraSystem& doraSys);
public:
    DefensePlayer(string n, int s) : Player(n, s) {}
    virtual Tile decideDiscard(const vector<Player*>& allPlayers) override;
    virtual CallType decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canMinkan) override;
};

class HighScorePlayer : public Player {
private:
    Rules rules;
    ShantenCounter sc;
    int evaluateHighScorePotential(const vector<Tile>& concealed, const vector<Meld>& melds, const DoraSystem& doraSys, int shanten);

public:
    HighScorePlayer(string n, int s) : Player(n, s) {}

    virtual Tile decideDiscard(const vector<Player*>& allPlayers) override;
    virtual CallType decideCall(const Tile& discardedTile, bool canChi, bool canPon, bool canMinkan) override;
};

#endif