
#include "ScoreStrategie.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include <chrono>
#include <sstream>

ScoreStrategie::ScoreStrategie(GameManager& gm, string nom)
    : gm{ gm },
    nom{ nom }
{
}

BT_Noeud::ETAT_ELEMENT ScoreStrategie::execute() noexcept {
    auto pre = std::chrono::high_resolution_clock::now();

    std::stringstream ss;
    ss << nom << std::endl;

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
        auto postCalcul = Minuteur::now();
        ss << "Dur�e calculerScoresEtCheminsTilesPourNpc = " << Minuteur::dureeMicroseconds(preCalcul, postCalcul) / 1000.f << "ms" << std::endl;

        // Choisir la meilleure tile pour ce npc et lui affecter son chemin
        auto preAffect = Minuteur::now();
        int tileChoisi = npc.affecterMeilleurChemin(manager.map);
        auto postAffect = Minuteur::now();
        ss << "Dur�e AffectationChemin = " << Minuteur::dureeMicroseconds(preAffect, postAffect) / 1000.f << "ms" << std::endl;

        // Mettre � jour les tilesAVisiter
        tilesAVisiter.push_back(tileChoisi);
    }

    // Temps d'execution
    auto post = Minuteur::now();
    ss << "Dur�e " << nom << " = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";

    GameManager::log(ss.str());
    return ETAT_ELEMENT::REUSSI;
}

void ScoreStrategie::calculerScore1Tile(int _tileID, Carte& _map, Npc& _npc, const std::vector<int>& _tilesAVisiter) {
    MapTile tile = _map.getTile(_tileID);
    // On ne consid�re la tile que si on ne la visite pas d�j� !
    if (tile.getStatut() == MapTile::Statut::CONNU && find(tilesAVisiter.begin(), tilesAVisiter.end(), tile.getId()) == tilesAVisiter.end()) {
        saveScore(tile, npc, tilesAVisiter);
    }
}

// Calcul le score de chaque tiles et son chemin pour un npc
// On prend en compte les tilesAVisiter des autres npcs pour que les tiles soient loins les unes des autres
void ScoreStrategie::calculerScoresTilesPourNpc(Npc& _npc, const std::vector<int>& _tilesAVisiter) noexcept {
    std::stringstream ss;
    ss << "Taille ensemble : " << _npc.getEnsembleAccessible().size();
    GameManager::log(ss.str());
    for (auto score : _npc.getEnsembleAccessible()) { // parcours toutes les tiles d�couvertes par l'ensemble des npcs et qui sont accessibles
        calculerScore1Tile(score.tuileID, manager.map, _npc, _tilesAVisiter);
    }
}

