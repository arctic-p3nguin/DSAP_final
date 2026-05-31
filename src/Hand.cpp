#include "Hand.hpp"

using namespace std;

void Hand::addTile(Tile t) {
    concealedTiles.push_back(t);
}

void Hand::sortHand() {
    sort(concealedTiles.begin(), concealedTiles.end());
}

void Hand::clear() {
    concealedTiles.clear();
    exposedMelds.clear();
}

Tile Hand::discardTile(int type, int val, int id) {
    for (auto it = concealedTiles.begin(); it != concealedTiles.end(); ++it) {
        if (it->getType() == type && it->getValue() == val && (id == -1 || it->getId() == id)) {
            Tile discarded = *it;
            concealedTiles.erase(it);
            return discarded;
        }
    }

    return Tile(-1, -1, -1, false);
}

void Hand::addMeld(Meld m) {
    exposedMelds.push_back(m);
}

const vector<Tile>& Hand::getConcealed() const {
    return concealedTiles;
}

const vector<Meld>& Hand::getMelds() const {
    return exposedMelds;
}