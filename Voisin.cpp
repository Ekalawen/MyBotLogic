#include "Voisin.h"

Voisin::Voisin(const int _voisinIndex)
    : tuileIndex{ _voisinIndex }
{
    etats.set();
}

int Voisin::getTuileIndex() const noexcept {
    return tuileIndex;
}

void Voisin::setEtat(const Etats etat, const bool val) noexcept {
    etats.set(etat, val);
}

bool Voisin::estEtat(const Etats etat) const noexcept {
    return etats[etat];
}