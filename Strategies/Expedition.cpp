
#include "Expedition.h"

Expedition::Expedition(GameManager gm) 
    : gm{ gm }
{
}

BT_Noeud::ETAT_ELEMENT Expedition::execute() {
    // Donc on sait ou se trouvent les objectifs
    // Mais on n'a pas de chemins qui nous relie à eux
    // Donc on va chercher à trouver ces chemins !

    // Pour ça chaque npc va visiter en premier les tuiles avec le plus haut score

    // L'ensemble des tiles que l'on va visiter
    vector<int> tilesAVisiter;

    for (auto& pair : gm.npcs) {
        Npc& npc = pair.second;
        npc.resetChemins();

        // Calculer le score de chaque tile pour le npc
        // En même temps on calcul le chemin pour aller à cette tile
        // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
        calculerScoresEtCheminsTiles(tilesAVisiter, npc);

        // Choisir la meilleure tile pour ce npc et lui affecter son chemin

        // Mettre à jour les tilesAVisiter
    }

    return ETAT_ELEMENT::REUSSI;
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void Expedition::calculerScoresEtCheminsTiles(vector<int> tilesAVisiter, Npc& npc) {
    for (auto pair : gm.m.tiles) {
        MapTile tile = pair.second;
        // On ne considère la tile que si on ne la visite pas déjà !
        if (find(tilesAVisiter.begin(), tilesAVisiter.end(), tile) != tilesAVisiter.end()) {
            saveScore(tile, npc);
        }
    }
}

// Le score est définit ici par plusieurs critères :
    // Critère nécessaire : la tuile est accessible par le npc
    // La distance du npc à la tuile
    // La distance moyenne de cette tile à tous les objectifs
    // Le degré d'intêret de la tuile. L'intérêt est définit par :
        // Le nombre de voisins inconnues de la tuile (on les connait pas et on peut passer)
        // Le nombre de voisins visibles de la tuile (on les connait pas et on peut juste voir)
    // La distance de cette tuile aux autres tuiles qui seront visités !
void Expedition::saveScore(MapTile tile, Npc& npc) {
    float score = 0;

    // Si le chemin entre le npc et la tile n'est pas accessible, on enregistre même pas le score de cette tile, elle est hors-jeu !
    Chemin cheminNpcTile = gm.m.aStar(npc.tileId, tile.id);
    if (!cheminNpcTile.isAccessible()) return;

    score += cheminNpcTile.distance() * COEF_DISTANCE_NPC_TILE;

    // On regarde la distance moyenne de cette tile à tous les objectifs
    float distanceMoyenne = 0;
    for (auto pair : gm.m.objectifs) {
        int objectif = pair.second.id;
        distanceMoyenne += gm.m.distanceHex(tile.id, objectif);
    }
    distanceMoyenne /= gm.m.objectifs.size();

    score += distanceMoyenne * COEF_DISTANCE_OBJECTIFS_TILE;
}
