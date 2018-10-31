#include "Mouvement.h"

Mouvement::Mouvement(const int npcID, const int tileSource, const int tileDestination, const Tile::ETilePosition direction)
: npcID(npcID), tileSource(tileSource), tileDestination(tileDestination), direction(direction)
{
}

void Mouvement::stop() noexcept {
    direction = Tile::CENTER;
    tileDestination = tileSource;
}

bool Mouvement::isNotStopped() const noexcept {
    return direction != Tile::CENTER;
}

int Mouvement::getNpcId() const noexcept {
    return npcID;
}

int Mouvement::getTileSource() const noexcept {
    return tileSource;
}

int Mouvement::getTileDestination() const noexcept {
    return tileDestination;
}

Tile::ETilePosition Mouvement::getDirection() const noexcept {
    return direction;
}
