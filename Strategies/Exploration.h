#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "MyBotLogic/GameManager.h"
#include "MyBotLogic/Strategies/ScoreStrategie.h"

class Exploration : public ScoreStrategie {
public:
    void saveScore(const MapTile& _tile, Npc& _npc, const std::vector<int>& _tilesAVisiter) const noexcept;
    float interet(const MapTile& _tile) const noexcept;

    enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit n�gatif
    enum { COEF_DISTANCE_TILE_AUTRE_TILES = 12 };
    enum { COEF_INTERET = 1 };
    enum { COEF_INTERET_ACCESSIBLE = 2 };
    enum { COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE = 1 };
    Exploration(GameManager&, std::string);
};

#endif
