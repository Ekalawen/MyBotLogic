#include "Voisin.h"

Voisin::Voisin(const int _tuileIndex)
    : tuileIndex{ _tuileIndex }
{}

int Voisin::getTuileIndex() const noexcept {
    return tuileIndex;
}