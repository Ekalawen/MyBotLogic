
#include "ScoreStrategie.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"

ScoreStrategie::ScoreStrategie(GameManager& gm, string nom)
    : gm{ gm },
    nom{ nom }
{
}

BT_Noeud::ETAT_ELEMENT ScoreStrategie::execute() {
    GameManager::Log(nom);
    // On ne sait pas où se trouvent les objectifs !
    // On va les chercher !

    // Pour ça chaque npc va visiter en premier les tuiles avec le plus haut score

    // L'ensemble des tiles que l'on va visiter
    vector<int> tilesAVisiter;

    for (auto& pair : gm.npcs) {
        Npc& npc = pair.second;
        npc.resetChemins();

        // Calculer le score de chaque tile pour le npc
        // En même temps on calcul le chemin pour aller à cette tile
        // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
        calculerScoresEtCheminsTilesPourNpc(npc, tilesAVisiter);

        // Choisir la meilleure tile pour ce npc et lui affecter son chemin
        int tileChoisi = npc.affecterMeilleurChemin();

        // Mettre à jour les tilesAVisiter
        tilesAVisiter.push_back(tileChoisi);
    }

    return ETAT_ELEMENT::REUSSI;
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void ScoreStrategie::calculerScoresEtCheminsTilesPourNpc(Npc& npc, vector<int> tilesAVisiter) {
    for (auto pair : gm.m.tiles) {
        MapTile tile = pair.second;
        // On ne considère la tile que si on ne la visite pas déjà !
        if (find(tilesAVisiter.begin(), tilesAVisiter.end(), tile.id) == tilesAVisiter.end()) {
            saveScore(tile, npc, tilesAVisiter);
        }
    }
}