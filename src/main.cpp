#include <iostream>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>  
#include <filesystem>

#include "GameManager.hpp"

using namespace std;
namespace fs = std::filesystem;

static string pad(double v, int width, int precision = 0) {
    ostringstream oss;
    oss << fixed << setprecision(precision) << setw(width) << v;
    return oss.str();
}

Player* createPlayer(const string& type, int id) {
    if (type == "defense") return new DefensePlayer("Defense", id);
    if (type == "speed") return new SpeedPlayer("Speed", id);
    if (type == "highscore") return new HighScorePlayer("HighScore", id);
    if (type == "shanten") return new ShantenPlayer("Shanten", id);
    if (type == "random") return new RandomPlayer("Random", id);
    return nullptr;
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));

    // 新增參數後，總共需要 7 個參數 + 執行檔本身 = 8
    if (argc != 8) {
        cout << "Usage:\n";
        cout << "mahjong_game.exe <round|match> <AI1> <AI2> <AI3> <AI4> <count> <output_csv_path>\n";
        cout << "Example:\n";
        cout << "mahjong_game.exe match defense speed highscore shanten 10000 out/out.csv\n";
        return 1;
    }

    string mode = argv[1];
    bool roundMode = (mode == "round");
    bool matchMode = (mode == "match");

    if (!roundMode && !matchMode) {
        cout << "Mode must be round or match.\n";
        return 1;
    }

    string aiTypes[4] = {argv[2], argv[3], argv[4], argv[5]};
    int TOTAL_SIM = stoi(argv[6]);
    string csvPath = argv[7]; // 獲取使用者輸入的 CSV 輸出路徑

    cout << "Mode: " << mode << "\n";
    cout << "Simulations: " << TOTAL_SIM << "\n";
    cout << "Output CSV: " << csvPath << "\n\n";

    // ===== MATCH MODE STATS =====
    long long finalScore[4] = {0};
    int rankCount[4][4] = {0};
    int tobiCount[4] = {0};
    double totalPt[4] = {0.0}; // 累加每位玩家獲得的段位 pt

    // ===== ROUND MODE STATS =====
    long long winPts[4] = {0};
    long long winTurn[4] = {0};
    int winCount[4] = {0};
    int dealIn[4] = {0};

    long long totalRounds = 0;

    int step = max(1, TOTAL_SIM / 10);

    // ======================================================
    // 雀魂段位公式參數（依據玉之間豪三精準值）
    // ======================================================
    const double BASE_SCORE = 25000.0;           // 原點基準
    const double UMA[4] = {70.0, 35.0, -5.0, -15.0}; // 總順位加成
    const double PENALTY = -80.0;               // 四位額外扣除的段位底分

    for (int i = 0; i < TOTAL_SIM; i++) {

        GameManager gm;

        for (int p = 0; p < 4; p++) {
            gm.addPlayer(createPlayer(aiTypes[p], p));
        }

        if (matchMode) {
            gm.initGameConfiguration(1, TOTAL_SIM);
            gm.runFullMatch();
        } else {
            gm.setOyaIndex(i % 4);

            gm.resetRound();

            while (gm.nextStep()) {}

            gm.roundEndSettlement();
        }

        // ===== STATS COLLECTION =====
        for (int p = 0; p < 4; p++) {
            winCount[p] += gm.getWinCount(p);
            dealIn[p] += gm.getDealInCount(p);
            winPts[p] += gm.getTotalWinPoints(p);
            winTurn[p] += gm.getTotalWinTurns(p);
        }

        if (matchMode) {

            totalRounds += gm.getRoundCount();

            vector<pair<int,int>> ranking;

            for (int p = 0; p < 4; p++) {
                int score = gm.getPlayerScore(p);
                finalScore[p] += score;
                ranking.push_back({score, p});

                if (score < 0) tobiCount[p]++;
            }

            sort(ranking.rbegin(), ranking.rend());

            for (int r = 0; r < 4; r++) {
                int playerIdx = ranking[r].second;
                rankCount[playerIdx][r]++;

                double matchPt = UMA[r] + ((double)ranking[r].first - BASE_SCORE) / 1000.0;
                
                if (r == 3) {
                    matchPt += PENALTY; 
                }
                
                totalPt[playerIdx] += matchPt;
            }

        } else {
            totalRounds += 1;
        }

        gm.clearPlayers();

        if ((i + 1) % step == 0) {
            cout << "Progress: " << i + 1 << "/" << TOTAL_SIM << "\n";
        }
    }

    cout << "\n==================== RESULTS ====================\n";

    // ======================================================
    // MATCH MODE OUTPUT (Console 輸出保持原樣)
    // ======================================================
    if (matchMode) {
        cout << "MATCH MODE RESULTS\n";
        cout << "Player | AvgScore | AvgPt   | WinRate  | DealIn   | AvgTurn | AvgRank | 1st | 2nd | 3rd | 4th\n";
        cout << "-------------------------------------------------------------------------------------------------\n";

        for (int p = 0; p < 4; p++) {

            double avgScore = (double)finalScore[p] / TOTAL_SIM;
            double avgPt = totalPt[p] / TOTAL_SIM; 
            double winRate = (totalRounds > 0) ? 100.0 * winCount[p] / totalRounds : 0;
            double dealRate = (totalRounds > 0) ? 100.0 * dealIn[p] / totalRounds : 0;
            
            // 計算平均和牌巡數 (和牌總巡數 / 和牌總次數)
            double avgTurn = (winCount[p] > 0) ? (double)winTurn[p] / winCount[p] : 0;
            
            // 計算平均順位: ((1位次數*1) + (2位次數*2) + (3位次數*3) + (4位次數*4)) / 總場數
            double avgRank = (double)(rankCount[p][0]*1 + rankCount[p][1]*2 + rankCount[p][2]*3 + rankCount[p][3]*4) / TOTAL_SIM;

            cout << setw(2) << p << "     | "
                << pad(avgScore, 8) << " | "
                << pad(avgPt, 7, 1) << " | " 
                << pad(winRate, 7, 2) << "% | "
                << pad(dealRate, 7, 2) << "% | "
                << pad(avgTurn, 7, 1) << " | "   // 新增
                << pad(avgRank, 7, 2) << " | "   // 新增
                << setw(3) << rankCount[p][0] << " | "
                << setw(3) << rankCount[p][1] << " | "
                << setw(3) << rankCount[p][2] << " | "
                << setw(3) << rankCount[p][3]
                << " (Tobi " << tobiCount[p] << ")"
                << "\n";
        }

        // ===== 寫入 CSV 檔案 =====
        try {
            fs::path p(csvPath);
            if (p.has_parent_path()) {
                fs::create_directories(p.parent_path());
            }

            ofstream csvFile(csvPath);
            if (csvFile.is_open()) {
                // 在 CSV 中多塞入兩個欄位：AvgTurn 與 AvgRank，方便 Python 讀取畫圖
                csvFile << "Player,AIName,AvgScore,AvgPt,WinRate,DealIn,AvgTurn,AvgRank,First,Second,Third,Fourth,Tobi,Mode\n";
                for (int p = 0; p < 4; p++) {
                    string aiName = argv[2 + p];
                    double avgScore = (double)finalScore[p] / TOTAL_SIM;
                    double avgPt = totalPt[p] / TOTAL_SIM;
                    double winRate = (totalRounds > 0) ? 100.0 * winCount[p] / totalRounds : 0;
                    double dealRate = (totalRounds > 0) ? 100.0 * dealIn[p] / totalRounds : 0;
                    double avgTurn = (winCount[p] > 0) ? (double)winTurn[p] / winCount[p] : 0;
                    double avgRank = (double)(rankCount[p][0]*1 + rankCount[p][1]*2 + rankCount[p][2]*3 + rankCount[p][3]*4) / TOTAL_SIM;

                    csvFile << p << "," << aiName << "," << avgScore << "," << avgPt << "," 
                            << winRate << "," << dealRate << "," << avgTurn << "," << avgRank << ","
                            << rankCount[p][0] << "," << rankCount[p][1] << "," << rankCount[p][2] << "," 
                            << rankCount[p][3] << "," << tobiCount[p] << ",match\n";
                }
                csvFile.close();
                cout << "\n[Success] Data written to: " << csvPath << "\n";
            }
        } catch (const exception& e) {
            cout << "\n[Error] CSV write failed: " << e.what() << "\n";
        }
    }
    // ======================================================
    // ROUND MODE OUTPUT
    // ======================================================
    else {

        cout << "ROUND MODE RESULTS\n";
        cout << "Player | WinRate  | DealIn   | AvgWinPts | AvgTurn\n";
        cout << "---------------------------------------------------\n";

        for (int p = 0; p < 4; p++) {

            double winRate = (totalRounds > 0) ? 100.0 * winCount[p] / totalRounds : 0;
            double dealRate = (totalRounds > 0) ? 100.0 * dealIn[p] / totalRounds : 0;
            double avgPts = (winCount[p] > 0) ? (double)winPts[p] / winCount[p] : 0;
            double avgTurn = (winCount[p] > 0) ? (double)winTurn[p] / winCount[p] : 0;

            cout << setw(2) << p << "     | "
                << pad(winRate, 7, 2) << "% | "
                << pad(dealRate, 7, 2) << "% | "
                << pad(avgPts, 9, 0) << " | "
                << pad(avgTurn, 8, 1)
                << "\n";
        }
        try {
            fs::path p(csvPath);
            if (p.has_parent_path()) fs::create_directories(p.parent_path());

            ofstream csvFile(csvPath);
            if (csvFile.is_open()) {
                csvFile << "Player,AIName,AvgScore,AvgPt,WinRate,DealIn,AvgTurn,AvgRank,First,Second,Third,Fourth,Tobi,Mode\n";
                for (int p = 0; p < 4; p++) {
                    string aiName = argv[2 + p];
                    double winRate = (totalRounds > 0) ? 100.0 * winCount[p] / totalRounds : 0;
                    double dealRate = (totalRounds > 0) ? 100.0 * dealIn[p] / totalRounds : 0;
                    double avgPts = (winCount[p] > 0) ? (double)winPts[p] / winCount[p] : 0;
                    double avgTurn = (winCount[p] > 0) ? (double)winTurn[p] / winCount[p] : 0;

                    // 在 round 模式下，AvgPt 欄位用來存「單局打點」
                    csvFile << p << "," << aiName << ",0," << avgPts << "," << winRate << "," << dealRate << ","
                            << avgTurn << ",0,0,0,0,0,0,round\n";
                }
                csvFile.close();
                cout << "\n[Success] Data written to: " << csvPath << "\n";
            }
        } catch (const exception& e) {
            cout << "\n[Error] CSV write failed: " << e.what() << "\n";
        }
    }
    cout << "=================================================\n";
    return 0;
}