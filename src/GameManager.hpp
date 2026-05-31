#ifndef GAMEMANAGER_HPP
#define GAMEMANAGER_HPP

#include <vector>
#include <random>
#include "Player.hpp"
#include "Tile.hpp"
#include "Rules.hpp"
#include "ScoreCounter.hpp"

class GameManager {
private:
    std::vector<Player*> players;
    std::vector<Tile> deck;
    int currentTurn;
    bool skipDrawOnce;
    Rules rules;
    DoraSystem doraSys;

    int winnerIndex;
    int loserIndex;
    int winningScore;
    ScoreResult winningResult;

    int matchMode;
    int totalMatches;

    int oyaIndex;
    int kyoku;
    int honba;
    int riichiPool;

    // ===== 新增統計資料 =====
    int roundCount;
    int roundStepCount;
    int winningTurn;

    int winCounts[4];
    int dealInCounts[4];
    long long totalWinPoints[4];
    long long totalWinHans[4];
    long long totalWinTurns[4];

public:
    GameManager();

    void clearPlayers();
    void addPlayer(Player* p);
    int getPlayerScore(int idx) const;
    void setOyaIndex(int idx) { oyaIndex = idx; }

    void initGameConfiguration(int mode, int totalGames);
    void runFullMatch();
    void resetRound();
    void roundEndSettlement();

    void initDeck();
    void shuffleDeck();
    void dealTiles();
    void startGame();
    bool nextStep();
    void printGameState() const;

    void setWinnerLoser(int win, int lose) {
        winnerIndex = win;
        loserIndex = lose;
    }
    int getWinnerIndex() const { return winnerIndex; }
    int getLoserIndex() const { return loserIndex; }

    void setWinningScore(int score) {
        winningScore = score;
        winningResult.totalScore = score;
    }
    int getWinningScore() const { return winningScore; }

    void setWinningResult(const ScoreResult& res) {
        winningResult = res;
        winningScore = res.totalScore;
    }
    const ScoreResult& getWinningResult() const { return winningResult; }

    void setWinningTurn(int turn) { winningTurn = turn; }
    int getWinningTurn() const { return winningTurn; }

    // ===== 統計 getter =====
    int getRoundCount() const { return roundCount; }
    int getWinCount(int idx) const { return (idx >= 0 && idx < 4) ? winCounts[idx] : 0; }
    int getDealInCount(int idx) const { return (idx >= 0 && idx < 4) ? dealInCounts[idx] : 0; }
    long long getTotalWinPoints(int idx) const { return (idx >= 0 && idx < 4) ? totalWinPoints[idx] : 0; }
    long long getTotalWinHan(int idx) const { return (idx >= 0 && idx < 4) ? totalWinHans[idx] : 0; }
    long long getTotalWinTurns(int idx) const { return (idx >= 0 && idx < 4) ? totalWinTurns[idx] : 0; }
};

#endif // GAMEMANAGER_HPP