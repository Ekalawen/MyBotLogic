#ifndef SCORE_STRATEGIE_H
#define SCORE_STRATEGIE_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

class GameManager;
class Npc;
class Carte;
class MapTile;
class ScoreStrategie : public BT_Feuille {
protected:
    GameManager &manager;
    string nom;
    bool calculerScoresTilesPourNpc(Npc& _npc, const vector<int>& _tilesAVisiter) noexcept;
    bool calculerScore1Tile(int _tileID, Carte& _carte, Npc& _npc, const vector<int>& _tilesAVisiter);

public:
    ScoreStrategie(GameManager&, string nom);
    virtual ETAT_ELEMENT execute() noexcept override;
    virtual bool saveScore(const MapTile& _tile, Npc& _npc, const vector<int>& _tilesAVisiter) const noexcept = 0;
};

#endif
