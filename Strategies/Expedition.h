#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "../Strategies/ScoreStrategie.h"

class Expedition : public ScoreStrategie {
    void saveScore(MapTile tile, float cout, Npc& npc, vector<int> tilesAVisiter) noexcept;
    float interet(MapTile tile) noexcept;

public:
    enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_OBJECTIFS_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_TILE_AUTRE_TILES = 1 };
    enum { COEF_INTERET = 1 };
    enum { COEF_INTERET_ACCESSIBLE = 2 };
    enum { COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE = 1 };
    Expedition(GameManager&, string);
};

#endif