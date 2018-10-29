
#include "ScoreStrategie.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include <chrono>

ScoreStrategie::ScoreStrategie(GameManager& gm, string nom)
    : gm{ gm },
    nom{ nom }
{
}

BT_Noeud::ETAT_ELEMENT ScoreStrategie::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();

    GameManager::Log(nom);
    // On ne sait pas o� se trouvent les objectifs !
    // On va les chercher !

    // Pour �a chaque npc va visiter en premier les tuiles avec le plus haut score

    // L'ensemble des tiles que l'on va visiter
    vector<int> tilesAVisiter;

    for (auto& pair : gm.getNpcs()) {
        Npc& npc = pair.second;
        npc.resetChemins();

        // Calculer le score de chaque tile pour le npc
        // En m�me temps on calcul le chemin pour aller � cette tile
        // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
        auto preCalcul = std::chrono::high_resolution_clock::now();
        calculerScoresTilesPourNpc(npc, tilesAVisiter);
        auto postCalcul = std::chrono::high_resolution_clock::now();
        GameManager::Log("Dur�e calculerScoresEtCheminsTilesPourNpc = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postCalcul - preCalcul).count() / 1000.f) + "ms");


        // Choisir la meilleure tile pour ce npc et lui affecter son chemin
        auto preAffect = std::chrono::high_resolution_clock::now();
        int tileChoisi = npc.affecterMeilleurChemin(gm.m);
        auto postAffect = std::chrono::high_resolution_clock::now();
        GameManager::Log("Dur�e AffectationChemin = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(postAffect - preAffect).count() / 1000.f) + "ms");


        // Mettre � jour les tilesAVisiter
        tilesAVisiter.push_back(tileChoisi);
    }

    // Temps d'execution
    auto post = std::chrono::high_resolution_clock::now();
    GameManager::Log("Dur�e " + nom + " = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
    
    return ETAT_ELEMENT::REUSSI;
}

void ScoreStrategie::calculerScore1Tile(int tileID, Map& m, Npc& npc, const vector<int> tilesAVisiter) {
    MapTile tile = m.getTile(tileID);
    // On ne consid�re la tile que si on ne la visite pas d�j� !
    if (tile.getStatut() == MapTile::Statut::CONNU && find(tilesAVisiter.begin(), tilesAVisiter.end(), tile.getId()) == tilesAVisiter.end()) {
        saveScore(tile, npc, tilesAVisiter);
    }
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void ScoreStrategie::calculerScoresTilesPourNpc(Npc& npc, vector<int> tilesAVisiter) noexcept {
   GameManager::Log("Taille ensemble : " + to_string(npc.getEnsembleAccessible().size()));
    for (auto tileID : npc.getEnsembleAccessible()) { // parcours toutes les tiles d�couvertes par l'ensemble des npcs et qui sont accessibles
        calculerScore1Tile(tileID, gm.m, npc, tilesAVisiter);
    }
}

