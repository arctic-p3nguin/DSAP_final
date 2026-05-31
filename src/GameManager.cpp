#include "GameManager.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

GameManager::GameManager()
    : currentTurn(0), matchMode(0), totalMatches(1), oyaIndex(0), kyoku(0), honba(0), riichiPool(0) {
    skipDrawOnce = false;
    winnerIndex = -1;
    loserIndex = -1;
    winningScore = 0;
    winningResult = ScoreResult();

    roundCount = 0;
    roundStepCount = 0;
    winningTurn = 0;

    for (int i = 0; i < 4; i++) {
        winCounts[i] = 0;
        dealInCounts[i] = 0;
        totalWinPoints[i] = 0;
        totalWinHans[i] = 0;
        totalWinTurns[i] = 0;
    }
}

void GameManager::clearPlayers() {
    for (auto p : players) {
        delete p;
    }
    players.clear();
}

void GameManager::addPlayer(Player* p) {
    players.push_back(p);
}

int GameManager::getPlayerScore(int idx) const {
    if (idx >= 0 && idx < (int)players.size()) {
        return players[idx]->getScore();
    }
    return 0;
}

void GameManager::initGameConfiguration(int mode, int totalGames) {
    this->matchMode = mode;
    this->totalMatches = totalGames;
}

void GameManager::initDeck() {
    deck.clear();
    for (int t = 0; t < 4; t++) {
        int maxVal = (t == 3) ? 7 : 9;
        for (int v = 0; v < maxVal; v++) {
            for (int i = 0; i < 4; i++) {
                deck.emplace_back(t, v, i, false);
            }
        }
    }
}

void GameManager::shuffleDeck() {
    random_device rd;
    mt19937 g(rd());
    shuffle(deck.begin(), deck.end(), g);
}

void GameManager::dealTiles() {
    for (int round = 0; round < 13; round++) {
        for (auto& p : players) {
            p->drawTile(deck.back());
            deck.pop_back();
        }
    }
}

void GameManager::resetRound() {
    for (auto& p : players) {
        p->resetRoundState();
    }

    initDeck();
    shuffleDeck();
    dealTiles();

    setWinnerLoser(-1, -1);
    setWinningScore(0);
    winningResult = ScoreResult();
    roundStepCount = 0;
    winningTurn = 0;

    doraSys.indicators.clear();
    doraSys.uraIndicators.clear();
    doraSys.activeCount = 1;

    for (int i = 0; i < 5; i++) {
        doraSys.indicators.push_back(deck.back());
        deck.pop_back();

        doraSys.uraIndicators.push_back(deck.back());
        deck.pop_back();
    }

    for (size_t i = 0; i < players.size(); i++) {
        players[i]->setDealer((int)i == oyaIndex);
        players[i]->setIsRiichi(false);
        players[i]->setTemporalFuriten(false);
    }

    if (!doraSys.indicators.empty()) {
        for (size_t i = 0; i < players.size(); i++) {
            players[i]->decreaseValidTile(
                doraSys.indicators[0].getType(),
                doraSys.indicators[0].getValue(),
                1
            );
        }
    }

    for (auto& p : players) {
        p->setDoraSystem(doraSys);
    }

    currentTurn = oyaIndex;
    skipDrawOnce = false;
}

void GameManager::startGame() {
    oyaIndex = 0;
    kyoku = 0;
    honba = 0;
    riichiPool = 0;

    roundCount = 0;
    roundStepCount = 0;
    winningTurn = 0;
    for (int i = 0; i < 4; i++) {
        winCounts[i] = 0;
        dealInCounts[i] = 0;
        totalWinPoints[i] = 0;
        totalWinHans[i] = 0;
        totalWinTurns[i] = 0;
    }

    for (auto& p : players) {
        p->setScore(25000);
    }
    resetRound();
}

void GameManager::runFullMatch() {
    for (auto& p : players) {
        p->setScore(25000);
    }

    oyaIndex = 0;
    kyoku = 0;
    honba = 0;
    riichiPool = 0;

    roundCount = 0;
    roundStepCount = 0;
    winningTurn = 0;
    for (int i = 0; i < 4; i++) {
        winCounts[i] = 0;
        dealInCounts[i] = 0;
        totalWinPoints[i] = 0;
        totalWinHans[i] = 0;
        totalWinTurns[i] = 0;
    }

    bool matchOver = false;
    int settledRounds = 0;

    while (!matchOver) {
        resetRound();

        bool roundRunning = true;
        while (roundRunning) {
            roundRunning = nextStep();
        }

        roundEndSettlement();
        settledRounds++;
        roundCount++;

        bool tobi = false;
        for (size_t i = 0; i < players.size(); i++) {
            if (players[i]->getScore() < 0) {
                tobi = true;
                break;
            }
        }

        if (matchMode == 0) {
            matchOver = true;
        } else if (matchMode == 1) {
            if (tobi || settledRounds >= 4) {
                matchOver = true;
            }
        }
    }
}

void GameManager::roundEndSettlement() {
    int winIdx = winnerIndex;
    int loseIdx = loserIndex;

    bool isFlow = (winIdx == -1);

    if (!isFlow) {
        bool isOyaWin = (winIdx == oyaIndex);
        int honbaBonusTotal = honba * 300;

        // ===== 新增統計 =====
        winCounts[winIdx]++;
        totalWinPoints[winIdx] += winningResult.totalScore;
        totalWinHans[winIdx] += winningResult.han;
        totalWinTurns[winIdx] += winningTurn;
        if (loseIdx >= 0 && loseIdx < 4) {
            dealInCounts[loseIdx]++;
        }

        if (loseIdx == -1) {
            int collectedScore = 0;

            for (int i = 0; i < 4; i++) {
                if (i == winIdx) continue;

                int pay = 0;
                if (isOyaWin) {
                    pay = winningResult.koPay;
                } else {
                    if (i == oyaIndex) {
                        pay = winningResult.oyaPay;
                    } else {
                        pay = winningResult.koPay;
                    }
                }

                pay += honba * 100;
                players[i]->setScore(players[i]->getScore() - pay);
                collectedScore += pay;
            }

            players[winIdx]->setScore(players[winIdx]->getScore() + collectedScore + (riichiPool * 1000));
            riichiPool = 0;

        } else {
            int totalPay = winningResult.totalScore + honbaBonusTotal;
            players[loseIdx]->setScore(players[loseIdx]->getScore() - totalPay);
            players[winIdx]->setScore(players[winIdx]->getScore() + totalPay + (riichiPool * 1000));
            riichiPool = 0;
        }

        if (isOyaWin) {
            honba++;
        } else {
            oyaIndex = (oyaIndex + 1) % 4;
            if (oyaIndex == 0) {
                kyoku++;
            }
            honba = 0;
        }

    } else {
        oyaIndex = (oyaIndex + 1) % 4;
        if (oyaIndex == 0) {
            kyoku++;
        }
        honba++;
    }
}

bool GameManager::nextStep() {
    if (deck.empty()) {
        return false;
    }

    ++roundStepCount;

    Player* p = players[currentTurn];
    if (p->getHand().getConcealed().empty()) {
        return false;
    }

    Tile toDiscard = p->getHand().getConcealed().back();

    if (this->skipDrawOnce) {
        this->skipDrawOnce = false;
        ShantenCounter sc;
        ParsedStructure dummyStructure;
        (void)sc.calculate(p->getHand().getConcealed(), p->getHand().getMelds(), dummyStructure);
    } else {
        Tile drawn = deck.back();
        deck.pop_back();
        p->drawTile(drawn);

        bool isMenzen = p->getHand().getMelds().empty();
        ShantenCounter sc;
        ParsedStructure dummyStructure;
        int currentShanten = sc.calculate(p->getHand().getConcealed(), p->getHand().getMelds(), dummyStructure);

        if (rules.isWin(p->getHand().getConcealed(), p->getHand().getMelds())) {
            bool isHaitei = deck.empty();
            bool isDealer = p->getIsDealer();
            bool isRiichi = p->getIsRiichi();

            ScoreCounter scoreCounter;
            ScoreResult res = scoreCounter.calculateBestScore(
                p->getHand().getConcealed(), p->getHand().getMelds(),
                isRiichi, false, true,
                false, false, isHaitei, false,
                false, false, false, false,
                0, currentTurn, isDealer,
                doraSys
            );
            if (res.han > 0) {
                setWinnerLoser(currentTurn, -1);
                setWinningResult(res);
                setWinningTurn(roundStepCount);
                return false;
            }
        }

        if (p->getIsRiichi()) {
            toDiscard = p->getHand().getConcealed().back();
        } else {
            toDiscard = p->decideDiscard(players);
        }

        p->discardTile(toDiscard);

        for (size_t i = 0; i < players.size(); i++) {
            players[i]->decreaseValidTile(toDiscard.getType(), toDiscard.getValue(), 1);
        }

        if (isMenzen && !p->getIsRiichi() && deck.size() >= 4 && currentShanten == 0) {
            p->setIsRiichi(true);
            p->setScore(p->getScore() - 1000);
            riichiPool++;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (i == currentTurn) continue;
        Player* otherPlayer = players[i];

        vector<Meld> playerMeld = otherPlayer->getHand().getMelds();
        vector<Tile> simulatedHand = otherPlayer->getHand().getConcealed();
        simulatedHand.push_back(toDiscard);

        if (rules.isWin(simulatedHand, playerMeld)) {
            bool isHoutei = deck.empty();
            bool isOtherPlayerDealer = otherPlayer->getIsDealer();
            bool isOtherPlayerRiichi = otherPlayer->getIsRiichi();

            ScoreCounter scoreCounter;
            ScoreResult res = scoreCounter.calculateBestScore(
                simulatedHand, playerMeld,
                isOtherPlayerRiichi, false, false,
                false, false, false, isHoutei,
                false, false, false, false,
                0, i, isOtherPlayerDealer, doraSys
            );
            if (res.han > 0) {
                setWinnerLoser(i, currentTurn);
                setWinningResult(res);
                setWinningTurn(roundStepCount);
                return false;
            }
        }
    }

    int callingPlayer = -1;
    CallType triggeredCall = CallType::None;
    vector<Tile> chosenChiCombo;

    for (int step = 1; step < 4; step++) {
        int i = (currentTurn + step) % 4;
        int matchCount = 0;
        for (const auto& t : players[i]->getHand().getConcealed()) {
            if (t.getType() == toDiscard.getType() && t.getValue() == toDiscard.getValue()) {
                matchCount++;
            }
        }

        bool canPon = (matchCount >= 2);
        bool canMinkan = (matchCount == 3);

        CallType decision = players[i]->decideCall(toDiscard, false, canPon, canMinkan);
        if (decision == CallType::Pon || decision == CallType::Minkan) {
            callingPlayer = i;
            triggeredCall = decision;
            break;
        }
    }

    if (triggeredCall == CallType::None) {
        int nextPlayerIndex = (currentTurn + 1) % 4;
        vector<vector<Tile>> chiCombos = rules.getChiCombinations(players[nextPlayerIndex]->getHand().getConcealed(), toDiscard);
        bool canChi = !chiCombos.empty();

        CallType decision = players[nextPlayerIndex]->decideCall(toDiscard, canChi, false, false);
        if (decision == CallType::Chi && canChi) {
            callingPlayer = nextPlayerIndex;
            triggeredCall = CallType::Chi;
            chosenChiCombo = chiCombos[0];
        }
    }

    if (triggeredCall != CallType::None && callingPlayer != -1) {
        if (triggeredCall == CallType::Pon) {
            players[callingPlayer]->performPon(toDiscard);
            for (size_t i = 0; i < players.size(); i++) {
                players[i]->decreaseValidTile(toDiscard.getType(), toDiscard.getValue(), 2);
            }
            currentTurn = callingPlayer;
            this->skipDrawOnce = true;
        }
        else if (triggeredCall == CallType::Minkan) {
            players[callingPlayer]->performKan(toDiscard, MeldType::Minkan);
            doraSys.activeCount = min(doraSys.activeCount + 1, 5);
            if (!doraSys.indicators.empty() && doraSys.activeCount - 1 < (int)doraSys.indicators.size()) {
                for (size_t i = 0; i < players.size(); i++) {
                    players[i]->decreaseValidTile(
                        doraSys.indicators[doraSys.activeCount - 1].getType(),
                        doraSys.indicators[doraSys.activeCount - 1].getValue(),
                        1
                    );
                }
            }
            for (size_t i = 0; i < players.size(); i++) {
                players[i]->decreaseValidTile(toDiscard.getType(), toDiscard.getValue(), 3);
            }
            currentTurn = callingPlayer;
            this->skipDrawOnce = false;
        }
        else if (triggeredCall == CallType::Chi) {
            players[callingPlayer]->performChi(chosenChiCombo, toDiscard);
            const Meld& latestMeld = players[callingPlayer]->getHand().getMelds().back();
            for (const Tile& tile : latestMeld.tiles) {
                if (tile.getType() == toDiscard.getType() && tile.getValue() == toDiscard.getValue() && tile.getId() == toDiscard.getId()) {
                    continue;
                }
                for (size_t i = 0; i < players.size(); i++) {
                    players[i]->decreaseValidTile(tile.getType(), tile.getValue(), 1);
                }
            }
            currentTurn = callingPlayer;
            this->skipDrawOnce = true;
        }
        return true;
    }

    for (int i = 0; i < 4; i++) {
        if (i == currentTurn) continue;
        Player* otherPlayer = players[i];
        vector<Tile> waitingTiles = rules.getWaitingTiles(
            otherPlayer->getHand().getConcealed(),
            otherPlayer->getHand().getMelds()
        );
        for (const auto& wTile : waitingTiles) {
            if (wTile.getType() == toDiscard.getType() && wTile.getValue() == toDiscard.getValue()) {
                otherPlayer->setTemporalFuriten(true);
                break;
            }
        }
    }

    currentTurn = (currentTurn + 1) % 4;
    return true;
}

void GameManager::printGameState() const {
    // 保留空實作
}