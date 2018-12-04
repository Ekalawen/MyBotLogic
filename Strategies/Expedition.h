#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "../Strategies/ScoreStrategie.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

class MapTile;
class Npc;
class GameManager;
class Expedition : public ScoreStrategie {
public:
    bool saveScore(const MapTile& tile, Npc& npc, const vector<int>& tilesAVisiter) const noexcept;
    float interet(const MapTile& tile) const noexcept;

    enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_OBJECTIFS_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_TILE_AUTRE_TILES = 12 };
    enum { COEF_INTERET = 1 };
    enum { COEF_INTERET_ACCESSIBLE = 2 };
    enum { COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE = 1 };
    Expedition(GameManager&, string);
};

#endif