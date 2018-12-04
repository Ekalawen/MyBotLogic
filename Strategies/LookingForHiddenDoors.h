#ifndef LOOKING_FOR_HIDDEN_DOORS_H
#define LOOKING_FOR_HIDDEN_DOORS_H

#include "MyBotLogic/Strategies/ScoreStrategie.h"

#include <vector>
#include <string>
using std::vector;
using std::string;

class MapTile;
class Npc;
class GameManager;
class LookingForHiddenDoors : public ScoreStrategie {
public:
    bool saveScore(const MapTile& tile, Npc& npc, const vector<int>& tilesAVisiter) const noexcept;
    float interet(const MapTile& tile) const noexcept;

    enum { COEF_DISTANCE_NPC_TILE = -12 };
    enum { COEF_INTERET = 1 };
    enum { COEF_NB_MURS_TILE = 1 };
    LookingForHiddenDoors(GameManager&, string);
};

#endif
