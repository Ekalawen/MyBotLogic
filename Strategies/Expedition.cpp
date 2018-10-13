
#include "Expedition.h"

Expedition::Expedition(GameManager gm) 
    : gm{ gm }
{
}

BT_Noeud::ETAT_ELEMENT Expedition::execute() {
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
        calculerScoresEtCheminsTiles(tilesAVisiter, npc);

        // Choisir la meilleure tile pour ce npc et lui affecter son chemin

        // Mettre � jour les tilesAVisiter
    }

    return ETAT_ELEMENT::REUSSI;
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void Expedition::calculerScoresEtCheminsTiles(vector<int> tilesAVisiter, Npc& npc) {
    for (auto pair : gm.m.tiles) {
        MapTile tile = pair.second;
        // On ne consid�re la tile que si on ne la visite pas d�j� !
        if (find(tilesAVisiter.begin(), tilesAVisiter.end(), tile) != tilesAVisiter.end()) {
            saveScore(tile, npc);
        }
    }
}

// Le score est d�finit ici par plusieurs crit�res :
    // Crit�re n�cessaire : la tuile est accessible par le npc
    // La distance du npc � la tuile
    // La distance moyenne de cette tile � tous les objectifs
    // Le degr� d'int�ret de la tuile. L'int�r�t est d�finit par :
        // Le nombre de voisins inconnues de la tuile (on les connait pas et on peut passer)
        // Le nombre de voisins visibles de la tuile (on les connait pas et on peut juste voir)
    // La distance de cette tuile aux autres tuiles qui seront visit�s !
void Expedition::saveScore(MapTile tile, Npc& npc) {
    float score = 0;

    // Si le chemin entre le npc et la tile n'est pas accessible, on enregistre m�me pas le score de cette tile, elle est hors-jeu !
    Chemin cheminNpcTile = gm.m.aStar(npc.tileId, tile.id);
    if (!cheminNpcTile.isAccessible()) return;

    score += cheminNpcTile.distance() * COEF_DISTANCE_NPC_TILE;

    // On regarde la distance moyenne de cette tile � tous les objectifs
    float distanceMoyenne = 0;
    for (auto pair : gm.m.objectifs) {
        int objectif = pair.second.id;
        distanceMoyenne += gm.m.distanceHex(tile.id, objectif);
    }
    distanceMoyenne /= gm.m.objectifs.size();

    score += distanceMoyenne * COEF_DISTANCE_OBJECTIFS_TILE;
}
