
#include "Exploration.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include "MyBotLogic/MapTile.h"

#include <string>
#include <vector>

using std::vector;

Exploration::Exploration(GameManager& _manager, string _nom)
    : ScoreStrategie(_manager, _nom)
{
}

// Le score est d�finit ici par plusieurs crit�res :
    // Crit�re n�cessaire : la tuile est accessible par le npc
    // La distance du npc � la tuile
    // La distance moyenne de cette tuile aux autres tuiles qui seront visit�s !
    // Le degr� d'int�ret de la tuile. 
void Exploration::saveScore(const MapTile& _tile, Npc& _npc, const vector<int>& _tilesAVisiter) const noexcept {
    float score = 0;

    // Si on a d�j� visit� cette case, son score est nul
    //if (tile.statut == MapTile::Statut::VISITE) return; // Appel� que si statut CONNU => non n�cessaire

    // On enregistre le cout, cad la distanc npc-tile
    score += _npc.distanceToTile(_tile.getId()) * COEF_DISTANCE_NPC_TILE;

    // On regarde l'int�ret de cette tile
    float interetTile = interet(_tile);
    score += interetTile * COEF_INTERET;
    if (interetTile == 0) return; // Si pas d'int�ret, la tile ne nous int�resse pas !

    // On regarde la distance moyenne de cette tuile aux autres tuiles d�j� visit�s
    if (!_tilesAVisiter.empty()) {
        float distanceMoyenneTiles = 0;
        for (auto autre : _tilesAVisiter) {
            distanceMoyenneTiles += manager.c.distanceHex(_tile.getId(), autre);
        }
        distanceMoyenneTiles /= _tilesAVisiter.size();
        score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
    }

    // Il reste � affecter le score et le chemin au npc
    _npc.addScore({ _tile.getId(), score });
}

// L'int�r�t est d�finit par :
    // Le nombre de voisins inconnues accessibles
    // Le nombre de voisins inconnues non accessibles MAIS visibles !
float Exploration::interet(const MapTile& tile) const noexcept {
    float interet = 0;

    int nbInconnuesAccessibles = 0;
    int nbInconnuesNonAccessiblesMaisVisibles = 0;
    for (auto voisinID : tile.getVoisinsIDParEtat(Etats::MYSTERIEUX)) {
        // Si autre est accessible ...
        if (tile.isVoisinAvecEtat(Etats::ACCESSIBLE, voisinID)) {
            ++nbInconnuesAccessibles;
            // Si autre est inaccessible ...
        }
        else {
            // Mais visible ...
            if (tile.isVoisinAvecEtat(Etats::VISIBLE, voisinID)) {
                ++nbInconnuesNonAccessiblesMaisVisibles;
            }
        }
    }

    interet += nbInconnuesAccessibles * COEF_INTERET_ACCESSIBLE;
    interet += nbInconnuesNonAccessiblesMaisVisibles * COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE;

    return interet;
}
