
#include "Expedition.h"

Expedition::Expedition(GameManager& gm) 
    : gm{ gm }
{
}

BT_Noeud::ETAT_ELEMENT Expedition::execute() {
    GameManager::Log("Appel de Expedition.execute()");
    // Donc on sait ou se trouvent les objectifs
    // Mais on n'a pas de chemins qui nous relie � eux
    // Donc on va chercher � trouver ces chemins !

    // Pour �a chaque npc va visiter en premier les tuiles avec le plus haut score

    // L'ensemble des tiles que l'on va visiter
    vector<int> tilesAVisiter;

    for (auto& pair : gm.npcs) {
        Npc& npc = pair.second;
        npc.resetChemins();

        // Calculer le score de chaque tile pour le npc
        // En m�me temps on calcul le chemin pour aller � cette tile
        // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
        calculerScoresEtCheminsTilesPourNpc(npc, tilesAVisiter);

        // Choisir la meilleure tile pour ce npc et lui affecter son chemin
        int tileChoisi = npc.affecterMeilleurChemin();

        // Mettre � jour les tilesAVisiter
        tilesAVisiter.push_back(tileChoisi);
    }

    return ETAT_ELEMENT::REUSSI;
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void Expedition::calculerScoresEtCheminsTilesPourNpc(Npc& npc, vector<int> tilesAVisiter) {
    for (auto pair : gm.m.tiles) {
        MapTile tile = pair.second;
        // On ne consid�re la tile que si on ne la visite pas d�j� !
        if (find(tilesAVisiter.begin(), tilesAVisiter.end(), tile.id) == tilesAVisiter.end()) {
            saveScore(tile, npc, tilesAVisiter);
        }
    }
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
    Chemin cheminNpcTile = gm.m.aStar(npc.tileId, tile.id);
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
            nbInconnuesAccessibles++;
        // Si autre est inaccessible ...
        } else {
            // Mais visible ...
            if (find(tile.voisinsVisibles.begin(), tile.voisinsVisibles.end(), autre) != tile.voisinsVisibles.end()) {
                nbInconnuesNonAccessiblesMaisVisibles++;
            }
        }
    }

    interet += nbInconnuesAccessibles * COEF_INTERET_ACCESSIBLE;
    interet += nbInconnuesNonAccessiblesMaisVisibles * COEF_INTERET_INACCESSIBLE_MAIS_VISIBLE;

    return interet;
}
