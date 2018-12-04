#ifndef CHECKING_HIDDEN_DOORS_H
#define CHECKING_HIDDEN_DOORS_H

#include "../BehaviorTree/BT_Feuille.h"

#include <vector>
#include <string>
using std::vector;
using std::string;

class MapTile;
class Npc;
class GameManager;
class Carte;
class CheckingHiddenDoors : public BT_Feuille {
   GameManager& manager;

public:
    CheckingHiddenDoors(GameManager& _manager) : manager{ _manager } {}
    ETAT_ELEMENT execute() noexcept override;

    enum { COEF_DISTANCE_NPC_TILE = -12 };
    enum { COEF_INTERET = 1 };
    enum { COEF_NB_MURS_TILE = 1 };

private:
    bool isAdajacentToWallToCheck(const int npcTileID) const noexcept; // Permet de savoir si un npc est sur une case sur laquelle il peut checker des murs
    void affecterDoorACheckerAuNpc(Npc& npc); // Permet de spécifier au Npc qu'il doit checker une porte, et la direction dans laquelle il doit regarder
    void affecterCheminNpcPourCheckerDoors(Npc& npc); // Affecte le chemin opti pour que le npc se déplace vers une case où il pourra chercker des murs
    void calculerScoresTilesPourNpc(Npc& _npc) noexcept;
    void calculerScore1Tile(int _tileID, Carte& _carte, Npc& _npc);
    void saveScore(const MapTile& _tile, Npc& _npc) const noexcept;
    float interet(const MapTile& tile) const noexcept;
};

#endif
