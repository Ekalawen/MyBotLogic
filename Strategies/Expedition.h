
#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"

class Expedition : public BT_Noeud {
    GameManager gm;
    void calculerScoresEtCheminsTiles(vector<int> tilesAVisiter, Npc& npc);
    void saveScore(MapTile tile, Npc& npc);

public:
    enum { COEF_DISTANCE_NPC_TILE = 1 };
    enum { COEF_DISTANCE_OBJECTIFS_TILE = 1 };
    Expedition(GameManager);
    ETAT_ELEMENT execute() override;
};

#endif