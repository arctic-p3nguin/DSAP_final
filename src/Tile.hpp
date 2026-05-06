#ifndef TILE_HPP 
#define TILE_HPP  
#include <iostream>

const int TILE_COUNT = 34;
const int TYPE_COUNT = 4;
const int VALUE_COUNT_N = 9;
const int VALUE_COUNT_Z = 7;

class Tile {
public:
    Tile(int type, int value, int id, bool isDora);

    bool isSameKind(const Tile& other);
    bool operator<(const Tile& other);

    int getType() const;
    int getValue() const;
    int getId() const;
    bool getIsDora() const;

private:
    int type;
    int value;
    int id;
    bool isDora;
};

#endif 