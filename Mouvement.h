#pragma once
#include "Globals.h"

class Mouvement {
	int npcID;
	int tileSource;
	int tileDestination;
	Tile::ETilePosition direction;
public:
	Mouvement(int npcID, int tileSource, int tileDestination, Tile::ETilePosition direction);

    void stop(); // Arr�te le mouvement
    bool isNotStopped();

    int getNpcId();
    int getTileSource();
    int getTileDestination();
    Tile::ETilePosition getDirection();
};
