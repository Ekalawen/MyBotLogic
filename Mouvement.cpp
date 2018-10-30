#include "Mouvement.h"

Mouvement::Mouvement(int _npcID, int _tileSource, int _tileDestination, Tile::ETilePosition _direction)
: npcID(_npcID), tileSource(_tileSource), tileDestination(_tileDestination), direction(_direction)
{
}

void Mouvement::stop() {
    direction = Tile::CENTER;
    tileDestination = tileSource;
}

bool Mouvement::isNotStopped() {
    return direction != Tile::CENTER;
}

int Mouvement::getNpcId() {
    return npcID;
}

int Mouvement::getTileSource() {
    return tileSource;
}

int Mouvement::getTileDestination() {
    return tileDestination;
}

Tile::ETilePosition Mouvement::getDirection() {
    return direction;
}
