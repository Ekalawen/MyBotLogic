#pragma once
#include "Globals.h"

class Mouvement {
	int npcID;
	int tileSource;
	int tileDestination;
	Tile::ETilePosition direction;
public:
	Mouvement(const int _npcID, const int _tileSource, const int _tileDestination, const Tile::ETilePosition _direction);

    void stop() noexcept; // Arr�te le mouvement
    bool isNotStopped() const noexcept;

    int getNpcId() const noexcept;
    int getTileSource() const noexcept;
    int getTileDestination() const noexcept;
    Tile::ETilePosition getDirection() const noexcept;
};
