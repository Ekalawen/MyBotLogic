
#include "LookingForHiddenDoors.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include "MyBotLogic/MapTile.h"

#include <string>
#include <vector>
using std::vector;

LookingForHiddenDoors::LookingForHiddenDoors(GameManager& _manager, string _nom)
    : ScoreStrategie(_manager, _nom)
{
}

// Le score est définit ici par plusieurs critéres :
    // Critére nécessaire : la tuile possède des murs adjacents
    // La distance du npc à la tuile
    // Le nombre de murs que possède la tuile
bool LookingForHiddenDoors::saveScore(const MapTile& _tile, Npc& _npc, const vector<int>& _tilesAVisiter) const noexcept {
    float score = 0;

    // On enregistre le cout, cad la distanc npc-tile
    score += _npc.distanceToTile(_tile.getId()) * COEF_DISTANCE_NPC_TILE;

    // On regarde l'intéret de cette tile
    float interetTile = interet(_tile);
    score += interetTile * COEF_INTERET;
    if (interetTile == 0) return false; // Si pas d'intéret, la tile ne nous intéresse pas !

    // Il reste à affecter le score et le chemin au npc
    _npc.addScore({ _tile.getId(), score });
    return true;
}

// L'intérét est définit par :
    // Le nombre de murs que possède la tuile
float LookingForHiddenDoors::interet(const MapTile& tile) const noexcept {
    float interet = 0;

    // On compte le nombre de murs
    int nbMursAdjacents = 0;
    for (Voisin voisin : tile.getVoisins()) {
        int voisinID = voisin.getTuileIndex();
        if (manager.c.hasWallBetweenUnchecked(tile.getId(), voisinID))
            nbMursAdjacents++;
    }

    interet += nbMursAdjacents * COEF_NB_MURS_TILE;;

    return interet;
}
