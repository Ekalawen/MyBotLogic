
#include "Exploration.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"

Exploration::Exploration(GameManager& _manager, std::string _nom)
    : ScoreStrategie(_manager, _nom)
{
}

// Le score est définit ici par plusieurs critères :
    // Critère nécessaire : la tuile est accessible par le npc
    // La distance du npc à la tuile
    // La distance moyenne de cette tuile aux autres tuiles qui seront visités !
    // Le degré d'intêret de la tuile. 
void Exploration::saveScore(MapTile _tile, Npc& _npc, std::vector<int> _tilesAVisiter) noexcept {
    float score = 0;

    // Si on a déjà visité cette case, son score est nul
    //if (tile.statut == MapTile::Statut::VISITE) return; // Appelé que si statut CONNU => non nécessaire

    // On enregistre le cout, cad la distanc npc-tile
    score += _npc.distanceToTile(_tile.getId()) * COEF_DISTANCE_NPC_TILE;

    // On regarde l'intêret de cette tile
    float interetTile = interet(_tile);
    score += interetTile * COEF_INTERET;
    if (interetTile == 0) return; // Si pas d'intêret, la tile ne nous intéresse pas !

    // On regarde la distance moyenne de cette tuile aux autres tuiles déjà visités
    if (!_tilesAVisiter.empty()) {
        float distanceMoyenneTiles = 0;
        for (auto autre : _tilesAVisiter) {
            distanceMoyenneTiles += manager.map.distanceHex(_tile.getId(), autre);
        }
        distanceMoyenneTiles /= _tilesAVisiter.size();
        score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
    }

    // Il reste à affecter le score et le chemin au npc
    _npc.addScore(_tile.getId(), score);
}

// L'intérêt est définit par :
    // Le nombre de voisins inconnues accessibles
    // Le nombre de voisins inconnues non accessibles MAIS visibles !
float Exploration::interet(MapTile _tile) noexcept {
    float interet = 0;

    int nbInconnuesAccessibles = 0;
    int nbInconnuesNonAccessiblesMaisVisibles = 0;
    for (auto autre : _tile.getVoisinsMysterieux()) {
        // Si autre est accessible ...
        if(_tile.isInVoisinsAccessibles(autre)) {
            ++nbInconnuesAccessibles;
        // Si autre est inaccessible ...
        } else {
            // Mais visible ...
            if(_tile.isInVoisinsVisibles(autre)) {
                ++nbInconnuesNonAccessiblesMaisVisibles;
            }
        }
    }

    interet += nbInconnuesAccessibles * COEF_INTERET_ACCESSIBLE;
    interet += nbInconnuesNonAccessiblesMaisVisibles * COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE;

    return interet;
}
