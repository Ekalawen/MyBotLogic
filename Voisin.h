#ifndef VOISIN_H
#define VOISIN_H

#include <vector>

class Voisin {
    int tuileIndex;

public:
    bool estVisible;
    bool estAccessible;
    bool estMysterieux;

    Voisin(const int _tuileIndex);

    int getTuileIndex() const noexcept;
};

#endif