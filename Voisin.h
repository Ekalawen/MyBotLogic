#ifndef VOISIN_H
#define VOISIN_H

#include <bitset>

enum Etats { VISIBLE, ACCESSIBLE, MYSTERIEUX, TOTAL };

class Voisin {
    //chaque bit représente un booléan, les états commencent a 1
    std::bitset<Etats::TOTAL> etats;
    int tuileIndex;

public:
    Voisin(const int voisinIndex);
    ~Voisin() = default;

    int getTuileIndex() const noexcept;
    void setEtat(const Etats etat, const bool val) noexcept;
    bool estEtat(const Etats etat) const noexcept;
};

#endif



