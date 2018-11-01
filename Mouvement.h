#pragma once
#include "Globals.h"

class Mouvement {
	int npcID;
	int tileSource;
	int tileDestination;
	Tile::ETilePosition direction;
public:
	Mouvement(const int npcID, const int tileSource, const int tileDestination, const Tile::ETilePosition direction);

    void stop() noexcept; // Arrête le mouvement
    bool isNotStopped() const noexcept;

    int getNpcId() const noexcept;
    int getTileSource() const noexcept;
    int getTileDestination() const noexcept;
    Tile::ETilePosition getDirection() const noexcept;
};
