#pragma once
#include "Globals.h"

class Mouvement
{
public:
	Mouvement(int npcID, int tileSource, int tileDestination, Tile::ETilePosition direction);

	int npcID;
	int tileSource;
	int tileDestination;
	Tile::ETilePosition direction;
};
