#ifndef VOISIN_H
#define VOISIN_H

#include <bitset>

class EtatNonDefini {};
enum Etats { VISIBLE, ACCESSIBLE, MYSTERIEUX, TOTAL };

class Voisin {
    std::bitset<Etats::TOTAL> etats;
    int tuileIndex;

public:
    Voisin(const int voisinIndex);
    ~Voisin() = default;

    int getTuileIndex() const;
    void setEtat(const Etats etat, const bool val);
    bool estEtat(const Etats etat) const;
};

#endif



