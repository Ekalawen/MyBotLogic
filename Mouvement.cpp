#include "Mouvement.h"



Mouvement::Mouvement(int npcID, int tileSource, int tileDestination, Tile::ETilePosition direction)
: npcID(npcID), tileSource(tileSource), tileDestination(tileDestination), direction(direction)
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
