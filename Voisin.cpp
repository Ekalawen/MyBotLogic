#include "Voisin.h"

Voisin::Voisin(const int _voisinIndex)
    : tuileIndex{ _voisinIndex }, etats{ 0x111 }
{}

int Voisin::getTuileIndex() const {
    return tuileIndex;
}

void Voisin::setEtat(const Etats etat, const bool val) {
    etats.set(etat, val);
}

bool Voisin::estEtat(const Etats etat) const {
    return etats[etat];
}