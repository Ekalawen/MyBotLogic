#include "Mouvement.h"



Mouvement::Mouvement(int npcID, int tileSource, int tileDestination, Tile::ETilePosition direction)
: npcID(npcID), tileSource(tileSource), tileDestination(tileDestination), direction(direction)
{
}
