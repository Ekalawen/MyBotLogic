#include "Mouvement.h"

Mouvement::Mouvement(const int _npcID, const int _tileSource, const int _tileDestination, const Tile::ETilePosition _direction)
: npcID{_npcID}, tileSource{_tileSource}, tileDestination{_tileDestination}, direction{_direction}
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
