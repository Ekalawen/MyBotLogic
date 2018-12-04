#pragma once
#include "Globals.h"
#include "Npc.h"
#include <vector>
using std::map;

class Carte;
class Mouvement {
	int npcID;
	int tileSource;
	int tileDestination;
	Tile::ETilePosition direction;
    bool isActivateDoorMouvement = false;
    Tile::ETilePosition directionActivateDoorMouvement = Tile::CENTER;
    bool isCheckingDoor = false;
    Tile::ETilePosition directionCheckingDoor = Tile::CENTER;

public:
	Mouvement(const int npcID, const int tileSource, const int tileDestination, const Tile::ETilePosition direction);

    void stop() noexcept; // Arrête le mouvement
    bool isNotStopped() const noexcept;

    int getNpcId() const noexcept;
    int getTileSource() const noexcept;
    int getTileDestination() const noexcept;
    Tile::ETilePosition getDirection() const noexcept;
    void setActivateDoor(Tile::ETilePosition direction) noexcept; // Permet de spécifier que ce mouvement a en fait pour but d'ouvrir une porte et non pas et déplacer le npc !
    void setCheckingDoor(Tile::ETilePosition direction) noexcept; // Permet de spécifier que ce mouvement a en fait pour but de checker un mur et non de déplacer le npc !

    void apply(vector<Action*>& actionList, map<int, Npc>& npcs, Carte& c) const noexcept; // Applique un mouvement au modèle
};
