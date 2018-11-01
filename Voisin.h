#ifndef VOISIN_H
#define VOISIN_H

#include "Globals.h"

#include <bitset>

enum Etats { VISIBLE, ACCESSIBLE, MYSTERIEUX, TOTAL };

class Voisin {
    int tuileIndex;
    Tile::ETilePosition direction;
    //chaque bit représente un booléan, les états commencent a 1
    std::bitset<Etats::TOTAL> etats;

public:
    Voisin(const int voisinIndex, Tile::ETilePosition direction);
    ~Voisin() = default;

    int getTuileIndex() const noexcept;
    Tile::ETilePosition getDirection() const noexcept;

    void setEtat(const Etats etat, const bool val) noexcept;
    bool estEtat(const Etats etat) const noexcept;
};

#endif



