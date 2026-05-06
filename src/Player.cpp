#include "Player.hpp"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

Player::Player(string n, int s) 
    : name(n), seat(s), isDealer(false), isRiichi(false) {}

void Player::drawTile(const Tile& tile) {
    hand.addTile(tile);
}

Tile Player::discardTile(const Tile& tile) {
    Tile discarded = hand.discardTile(tile.getType(), tile.getValue(), tile.getId());
    river.push_back(discarded);
    return discarded;
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
    for (int i = 0; i < 2; ++i) {
        hand.discardTile(discardedTile.getType(), discardedTile.getValue(), discardedTile.getId());
    }
    vector<Tile> meldContents(3, discardedTile);
    hand.addMeld({MeldType::Pon, meldContents});
}

void Player::performKan(const Tile& discardedTile, MeldType kanType) {
    vector<Tile> meldContents;
    int removeCount = (kanType == MeldType::Ankan) ? 4 : 3;
    for (int i = 0; i < removeCount; ++i) {
        hand.discardTile(discardedTile.getType(), discardedTile.getValue(), discardedTile.getId());
    }
    meldContents.assign(4, discardedTile);
    hand.addMeld({kanType, meldContents});
}

void Player::displayStatus() const {
    cout << "[" << name << " Seated " << seat << "]" << (isDealer ? " *Dealer*" : "") << "\n";
    cout << "River Size: " << river.size() << "\n";
}

Tile RandomPlayer::decideDiscard() {
    const vector<Tile>& currentTiles = hand.getConcealed();
    
    int randomIndex = rand() % currentTiles.size();
    return currentTiles[randomIndex];
}

Tile ShantenPlayer::decideDiscard() {
    const vector<Tile>& curTiles = hand.getConcealed();
    const vector<Meld>& curMelds = hand.getMelds();
    int bestShanten = 8;
    Tile toDiscard = curTiles.back(); 
    vector<Tile> tempTiles = curTiles;
    // 枚舉丟手上的牌 然後看哪一張丟出去的向聽數會最低就選他
    for(int i = 0; i < curTiles.size(); i++) {
        Tile tempDiscard = tempTiles[i];
        tempTiles.erase(tempTiles.begin() + i);

        int shanten = sc.calculate(tempTiles, curMelds);
        if(shanten < bestShanten) {
            bestShanten = shanten;
            toDiscard = tempDiscard;
        }

        tempTiles.insert(tempTiles.begin() + i, tempDiscard);

        if(bestShanten <= 0) break;
    }
    return toDiscard;
}