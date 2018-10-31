
#include "Npc.h"
#include "Globals.h"
#include "Voisin.h"
#include "GameManager.h"
#include <chrono>
#include <algorithm>
#include <sstream>

Npc::Npc(const NPCInfo info) :
	id{ static_cast<int>(info.npcID) },
	tileId{ static_cast<int>(info.tileID) },
	tileObjectif{ -1 },
	chemin{ Chemin{} },
	estArrive{ false }
{
}

void Npc::move(Tile::ETilePosition direction, Map &m) noexcept {
    tileId = m.getAdjacentTileAt(tileId, direction);
    m.getTile(tileId).setStatut(MapTile::Statut::VISITE);
}

void Npc::resetChemins() noexcept {
    cheminsPossibles.clear();
    scoresAssocies.clear();
}

void Npc::addChemin(Chemin& chemin) noexcept {
    cheminsPossibles.push_back(chemin);
}

void Npc::addScore(ScoreType _score) noexcept {
    scoresAssocies.emplace_back(std::move(_score));
}

Chemin Npc::getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept {
    Chemin cheminMin;
    cheminMin.setInaccessible();
    int distMin = tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); ++i) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas d�j� pris et qu'il est plus court !
		int destination = (chemin.empty()) ? tileId : chemin.destination(); // si le npc est d�j� arriv� il reste l�
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), destination) == objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;
}

int Npc::affecterMeilleurChemin(Map &m) noexcept {
    stringstream ss;

    if (scoresAssocies.empty()) {
        // Dans ce cas-l� on reste sur place !
        chemin = Chemin{};
        ss << "Le Npc " << id << " n'a rien a rechercher et reste sur place !";
        GameManager::Log(ss.str());
        return tileId;
    }
	
    // On cherche le meilleur score
    auto preScore = std::chrono::high_resolution_clock::now();
    auto bestIter = std::max_element(begin(scoresAssocies), end(scoresAssocies),
        [](const ScoreType& scoreDroite, const ScoreType& scoreGauche){
            return scoreDroite.score < scoreGauche.score;
        });
    auto postScore = std::chrono::high_resolution_clock::now();
    ss << "Dur�e chercher meilleur score = " << std::chrono::duration_cast<std::chrono::microseconds>(postScore - preScore).count() / 1000.f << "ms" <<std::endl;

    // On affecte son chemin, mais il nous faut le calculer ! =)
    auto preAStar = std::chrono::high_resolution_clock::now();
    chemin = m.aStar(tileId, bestIter->tuileID);
    auto postAStar = std::chrono::high_resolution_clock::now();

    ss << "Le Npc " << to_string(id) << " va rechercher la tile " << chemin.destination() << std::endl;
    ss << "Dur�e a* = " << std::chrono::duration_cast<std::chrono::microseconds>(postAStar - preAStar).count() / 1000.f << "ms" << std::endl;
    GameManager::Log(ss.str());

    // On renvoie la destination
    return chemin.destination();
}

void Npc::floodfill(Map &m) {
    ensembleAccessible.clear();

    vector<int> oldOpen;
    // On ajoute le noeud initial
    vector<int> newOpen { tileId };

    int cout = 0;
    // Tant qu'il reste des noeuds � traiter ...
    while (!newOpen.empty()) {
        oldOpen = newOpen;
        newOpen = vector<int>();
        // On regarde les voisins des dernieres tuiles ajout�es
        for (int tileID : oldOpen) {
            for (auto voisinID : m.getTile(tileID).getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
                // Si elle est connu
                if (m.getTile(voisinID).existe()) {
                    // Si elle n'est pas d�j� ajout�
                    if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&voisinID](const DistanceType& type) {
                        return type.tuileID == voisinID; }) == ensembleAccessible.end()) {
                        // On l'ajoute comme nouvelle tuile ouverte
                        newOpen.push_back(voisinID);
                    }
                }
            }

            // On d�finit les derni�res tuiles ajout�s avec leur co�t courant
            if (find_if(ensembleAccessible.begin(), ensembleAccessible.end(), [&](const DistanceType& type) {
                return type.tuileID == tileID; }) == ensembleAccessible.end()) {
                ensembleAccessible.emplace_back(tileID, cout);
            }
        }
        cout++;
    }
}


int Npc::getId() {
    return id;
}

int Npc::getTileId() {
    return tileId;
}

int Npc::getTileObjectif() {
    return tileObjectif;
}

void Npc::setTileObjectif(int idTile) {
    tileObjectif = idTile;
}

Chemin& Npc::getChemin() {
    return chemin;
}

Distances& Npc::getEnsembleAccessible() {
    return ensembleAccessible;
}

bool Npc::isAccessibleTile(int _tuileID) {
    return std::find_if(begin(ensembleAccessible), end(ensembleAccessible), [&_tuileID](const DistanceType& type) {
        return type.tuileID == _tuileID;
    }) != end(ensembleAccessible);
}

int Npc::distanceToTile(int _tuileID) {
    if (!isAccessibleTile(_tuileID))
        throw tile_inaccessible{};

    return std::find_if(begin(ensembleAccessible), end(ensembleAccessible), [&_tuileID](const DistanceType& type) {
        return type.tuileID == _tuileID;
    })->score;
}

bool Npc::isArrived() {
    return estArrive;
}

void Npc::setArrived(bool etat) {
    estArrive = etat;
}
