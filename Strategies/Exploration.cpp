
#include "Exploration.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"

Exploration::Exploration(GameManager& gm, string nom) 
    : ScoreStrategie(gm, nom)
{
}

// Le score est définit ici par plusieurs critères :
    // Critère nécessaire : la tuile est accessible par le npc
    // La distance du npc à la tuile
    // La distance moyenne de cette tuile aux autres tuiles qui seront visités !
    // Le degré d'intêret de la tuile. 
void Exploration::saveScore(MapTile tile, int cout, Npc& npc, vector<int> tilesAVisiter) noexcept {
    float score = 0;

    // Si on a déjà visité cette case, son score est nul
    if (tile.statut == MapTile::Statut::VISITE) return;

    // On enregistre le cout, cad la distanc npc-tile
    score += cout * COEF_DISTANCE_NPC_TILE;

    // On regarde l'intêret de cette tile
    float interetTile = interet(tile);
    score += interetTile * COEF_INTERET;
    if (interetTile == 0) return; // Si pas d'intêret, la tile ne nous intéresse pas !

    // On regarde la distance moyenne de cette tuile aux autres tuiles déjà visités
    if (!tilesAVisiter.empty()) {
        float distanceMoyenneTiles = 0;
        for (auto autre : tilesAVisiter) {
            distanceMoyenneTiles += gm.m.distanceHex(tile.id, autre);
        }
        distanceMoyenneTiles /= tilesAVisiter.size();
        score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
    }

    // Il reste à affecter le score et le chemin au npc
    npc.addScore(tile.id, score);
}

// L'intérêt est définit par :
    // Le nombre de voisins inconnues accessibles
    // Le nombre de voisins inconnues non accessibles MAIS visibles !
float Exploration::interet(MapTile tile) noexcept {
    float interet = 0;

    int nbInconnuesAccessibles = 0;
    int nbInconnuesNonAccessiblesMaisVisibles = 0;
    for (auto autre : tile.voisinsMysterious) {
        // Si autre est accessible ...
        if (find(tile.voisinsAccessibles.begin(), tile.voisinsAccessibles.end(), autre) != tile.voisinsAccessibles.end()) {
            ++nbInconnuesAccessibles;
        // Si autre est inaccessible ...
        } else {
            // Mais visible ...
            if (find(tile.voisinsVisibles.begin(), tile.voisinsVisibles.end(), autre) != tile.voisinsVisibles.end()) {
                ++nbInconnuesNonAccessiblesMaisVisibles;
            }
        }
    }

    interet += nbInconnuesAccessibles * COEF_INTERET_ACCESSIBLE;
    interet += nbInconnuesNonAccessiblesMaisVisibles * COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE;

    return interet;
}
