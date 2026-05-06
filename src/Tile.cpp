#include "Tile.hpp"

// type: 0~3 represent 萬、索、筒、字, value: 0~8(字只有0~6 東南西北白發中), id: 0~3
Tile::Tile(int type, int value, int id, bool isDora) {
    this->type = type;
    this->value = value;
    this->id = id;
    this->isDora = isDora;
}

bool Tile::isSameKind(const Tile& other) {
    return this->type == other.type && this->value == other.value;
}

bool Tile::operator<(const Tile& other) {
    if (this->type != other.type) return this->type < other.type;
    if (this->value != other.value) return this->value < other.value;
    return this->id < other.id;
}

int Tile::getType() const { return type; }
int Tile::getValue() const { return value; }
int Tile::getId() const { return id; }
bool Tile::getIsDora() const { return isDora; }