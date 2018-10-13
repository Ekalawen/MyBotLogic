
#ifndef EXPEDITION_H
#define EXPEDITION_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "MyBotLogic/GameManager.h"

class Expedition : public BT_Feuille {
    GameManager gm;
    void calculerScoresEtCheminsTilesPourNpc(Npc& npc, vector<int> tilesAVisiter);
    void saveScore(MapTile tile, Npc& npc, vector<int> tilesAVisiter);
    float interet(MapTile tile);

public:
    enum { COEF_DISTANCE_NPC_TILE = -12 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_OBJECTIFS_TILE = -1 }; // Il faut que ce soit négatif
    enum { COEF_DISTANCE_TILE_AUTRE_TILES = 1 };
    enum { COEF_INTERET = 1 };
    enum { COEF_INTERET_ACCESSIBLE = 2 };
    enum { COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE = 1 };
    Expedition(GameManager);
    ETAT_ELEMENT execute() override;
};

#endif