#include <iostream>
#include <chrono>

#include "GameManager.hpp"

using namespace std;
using namespace std::chrono;

Player* createPlayer(const string& type, int id) {
    if (type == "shanten") return new ShantenPlayer("Shanten", id);
    if (type == "random") return new RandomPlayer("Random", id);
    if (type == "defense") return new DefensePlayer("Defense", id);
    return nullptr;
}

double runTest(const string playerType[4], int rounds) {

    auto start = high_resolution_clock::now();

    for (int i = 0; i < rounds; i++) {

        GameManager gm;

        for (int p = 0; p < 4; p++) {
            gm.addPlayer(createPlayer(playerType[p], p));
        }

        gm.setOyaIndex(i % 4);

        gm.resetRound();

        while (gm.nextStep()) {}

        gm.roundEndSettlement();

        gm.clearPlayers();
    }

    auto end = high_resolution_clock::now();

    return duration<double>(end - start).count();
}

int main() {

    const int TEST_ROUNDS = 300;

    string shantenTeam[4] = {
        "shanten",
        "shanten",
        "shanten",
        "shanten"
    };

    string defenseTeam[4] = {
        "defense",
        "defense",
        "defense",
        "defense"
    };

    cout << "========================================\n";
    cout << "Algorithm Speed Test\n";
    cout << "Rounds per test = " << TEST_ROUNDS << "\n";
    cout << "========================================\n\n";

    double defenseTime = runTest(defenseTeam, TEST_ROUNDS);

    cout << "4 Defense Players\n";
    cout << "Elapsed Time: "
         << defenseTime
         << " seconds\n\n";

    double shantenTime = runTest(shantenTeam, TEST_ROUNDS);

    cout << "4 Shanten Players\n";
    cout << "Elapsed Time: "
         << shantenTime
         << " seconds\n\n";


    return 0;
}