#ifndef SCORE_STRATEGIE_H
#define SCORE_STRATEGIE_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "MyBotLogic/GameManager.h"

class ScoreStrategie : public BT_Feuille {
protected:
    GameManager &gm;
    string nom;
    void calculerScoresEtCheminsTilesPourNpc(Npc& npc, vector<int> tilesAVisiter);
    virtual void saveScore(MapTile tile, float cout, Npc& npc, vector<int> tilesAVisiter) = 0;

public:
    ScoreStrategie(GameManager&, string nom);
    virtual ETAT_ELEMENT execute() override;
};

#endif
