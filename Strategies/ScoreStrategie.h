#ifndef SCORE_STRATEGIE_H
#define SCORE_STRATEGIE_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "MyBotLogic/GameManager.h"

class ScoreStrategie : public BT_Feuille {
protected:
    GameManager &manager;
    std::string nom;
    void calculerScoresTilesPourNpc(Npc& _npc, const std::vector<int>& _tilesAVisiter) noexcept;
    void calculerScore1Tile(int _tileID, Carte& _map, Npc& _npc, const std::vector<int>& _tilesAVisiter);

public:
    ScoreStrategie(GameManager&, std::string _nom);
    virtual ETAT_ELEMENT execute() noexcept override;
    virtual void saveScore(const MapTile& _tile, Npc& _npc, const std::vector<int>& _tilesAVisiter) const noexcept = 0;
};

#endif
