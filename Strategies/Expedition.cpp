
#include "Expedition.h"
#include "MyBotLogic/MapTile.h"
#include "../GameManager.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

Expedition::Expedition(GameManager& _manager, string _nom)
    : ScoreStrategie(_manager, _nom)
{
}

// Le score est définit ici par plusieurs critères :
    // Critère nécessaire : la tuile est accessible par le npc
    // La distance du npc à la tuile
    // La distance moyenne de cette tile à tous les objectifs
    // La distance moyenne de cette tuile aux autres tuiles qui seront visités !
    // Le degré d'intèret de la tuile. 
void Expedition::saveScore(const MapTile& _tile, Npc& _npc, const vector<int>& _tilesAVisiter) const noexcept {
    // Précondition : tile.statut == CONNU
    float score = 0;

    // Si on a déjà visité cette case, son score est nul
    //if (tile.statut == MapTile::Statut::VISITE) return;

	// On regarde l'intéret de cette tile
	float interetTile = interet(_tile);
    if (interetTile == 0) return; // Si pas d'intéret, la tile ne nous intéresse pas !
	score += interetTile * COEF_INTERET;

    // On enregistre le cout, cad la distance npc-tile
    score += _npc.distanceToTile(_tile.getId()) * COEF_DISTANCE_NPC_TILE;

    // On regarde la distance moyenne de cette tile à tous les objectifs
    float distanceMoyenne = 0;
    for (auto objectif : manager.c.getObjectifs()) {
        distanceMoyenne += manager.c.distanceHex(_tile.getId(), objectif);
    }
    distanceMoyenne /= manager.c.getObjectifs().size();
    score += distanceMoyenne * COEF_DISTANCE_OBJECTIFS_TILE;

    // On regarde la distance moyenne de cette tuile aux autres tuiles déjà visités
    if (!_tilesAVisiter.empty()) {
        float distanceMoyenneTiles = 0;
        for (auto autre : _tilesAVisiter) {
            distanceMoyenneTiles += manager.c.distanceHex(_tile.getId(), autre);
        }
        distanceMoyenneTiles /= _tilesAVisiter.size();
        score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
    }

    // Il reste à affecter le score et le chemin au npc
    _npc.addScore({ _tile.getId(), score });
}

// L'intérét est définit par :
    // Le nombre de voisins inconnues accessibles
    // Le nombre de voisins inconnues non accessibles MAIS visibles !
float Expedition::interet(const MapTile& tile) const noexcept {
    float interet = 0;

    int nbInconnuesAccessibles = 0;
    int nbInconnuesNonAccessiblesMaisVisibles = 0;
    for (auto voisinID : tile.getVoisinsIDParEtat(Etats::MYSTERIEUX)) {
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
