
#include "Expedition.h"

Expedition::Expedition(GameManager& gm, string nom) 
    : ScoreStrategie(gm, nom)
{
}

// Le score est d�finit ici par plusieurs crit�res :
    // Crit�re n�cessaire : la tuile est accessible par le npc
    // La distance du npc � la tuile
    // La distance moyenne de cette tile � tous les objectifs
    // La distance moyenne de cette tuile aux autres tuiles qui seront visit�s !
    // Le degr� d'int�ret de la tuile. 
void Expedition::saveScore(MapTile tile, Npc& npc, vector<int> tilesAVisiter) {
    float score = 0;

    // Si le chemin entre le npc et la tile n'est pas accessible, on enregistre m�me pas le score de cette tile, elle est hors-jeu !
    Chemin cheminNpcTile = gm.m.WAStar(npc.tileId, tile.id);
    if (!cheminNpcTile.isAccessible()) return;
    score += cheminNpcTile.distance() * COEF_DISTANCE_NPC_TILE;

    // On regarde l'int�ret de cette tile
    float interetTile = interet(tile);
    score += interetTile * COEF_INTERET;
    if (interetTile == 0) return; // Si pas d'int�ret, la tile ne nous int�resse pas !

    // On regarde la distance moyenne de cette tile � tous les objectifs
    float distanceMoyenne = 0;
    for (auto pair : gm.m.objectifs) {
        int objectif = pair.second.id;
        distanceMoyenne += gm.m.distanceHex(tile.id, objectif);
    }
    distanceMoyenne /= gm.m.objectifs.size();
    score += distanceMoyenne * COEF_DISTANCE_OBJECTIFS_TILE;

    // On regarde la distance moyenne de cette tuile aux autres tuiles d�j� visit�s
    if (!tilesAVisiter.empty()) {
        float distanceMoyenneTiles = 0;
        for (auto autre : tilesAVisiter) {
            distanceMoyenneTiles += gm.m.distanceHex(tile.id, autre);
        }
        distanceMoyenneTiles /= tilesAVisiter.size();
        score += distanceMoyenneTiles * COEF_DISTANCE_TILE_AUTRE_TILES;
    }

    // Il reste � affecter le score et le chemin au npc
    npc.addCheminWithScore(cheminNpcTile, score);
}

// L'int�r�t est d�finit par :
    // Le nombre de voisins inconnues accessibles
    // Le nombre de voisins inconnues non accessibles MAIS visibles !
float Expedition::interet(MapTile tile) {
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
