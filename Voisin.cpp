#include "Voisin.h"

Voisin::Voisin(const int _voisinIndex, Tile::ETilePosition _direction)
    : tuileIndex{ _voisinIndex }, direction {_direction }
{
    etats.set();
}

int Voisin::getTuileIndex() const noexcept {
    return tuileIndex;
}

Tile::ETilePosition Voisin::getDirection() const noexcept {
    return direction;
}

void Voisin::setEtat(const Etats etat, const bool val) noexcept {
    etats.set(etat, val);
}

bool Voisin::estEtat(const Etats etat) const noexcept {
    return etats[etat];
}