#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "../BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"
#include "../Strategies/ScoreStrategie.h"
#include <vector>
#include <string>

class Expedition : public ScoreStrategie {
public:
    void saveScore(const MapTile& tile, Npc& npc, const std::vector<int>& tilesAVisiter) const noexcept;
    float interet(const MapTile& tile) const noexcept;

    enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_OBJECTIFS_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_TILE_AUTRE_TILES = 12 };
    enum { COEF_INTERET = 1 };
    enum { COEF_INTERET_ACCESSIBLE = 2 };
    enum { COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE = 1 };
    Expedition(GameManager&, std::string);
};

#endif