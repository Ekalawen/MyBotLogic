
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
	estArrive{ false }
{
}

void Npc::move(const Tile::ETilePosition _direction, Carte& _map) noexcept {
    tileId = _map.getAdjacentTileAt(tileId, _direction);
    _map.getTile(tileId).setStatut(MapTile::Statut::VISITE);
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

Chemin Npc::getCheminMinNonPris(const vector<int>& objectifsPris, const int tailleCheminMax) const noexcept {
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

int Npc::affecterMeilleurChemin(const Carte&_map) noexcept {
    std::stringstream ss;

    if (scoresAssocies.empty()) {
        // Dans ce cas-l� on reste sur place !
        chemin = Chemin{};
        ss << "Le Npc " << id << " n'a rien a rechercher et reste sur place !";
        GameManager::log(ss.str());
        return tileId;
    }
	
    // On cherche le meilleur score
    auto preScore = std::chrono::high_resolution_clock::now();
    auto bestIter = std::max_element(begin(scoresAssocies), end(scoresAssocies),
        [](const ScoreType& scoreDroite, const ScoreType& scoreGauche){
            return scoreDroite.score < scoreGauche.score;
        });
    auto postScore = Minuteur::now();
    ss << "Dur�e chercher meilleur score = " << Minuteur::dureeMicroseconds(preScore, postScore) / 1000.f << "ms" <<std::endl;
    

    // On affecte son chemin, mais il nous faut le calculer ! =)
    auto preAStar = std::chrono::high_resolution_clock::now();
    chemin = m.aStar(tileId, bestIter->tuileID);
    auto postAStar = std::chrono::high_resolution_clock::now();

    ss << "Le Npc " << std::to_string(id) << " va rechercher la tile " << chemin.destination() << std::endl;
    ss << "Dur�e a* = " << Minuteur::dureeMicroseconds(preAStar, postAStar) / 1000.f << "ms" << std::endl;
    GameManager::log(ss.str());

    // On renvoie la destination
    return chemin.destination();
}

void Npc::floodfill(const Carte& _map) {
void Npc::floodfill(const Map &m) {
    ensembleAccessible.clear();

    std::vector<int> oldOpen;
    // On ajoute le noeud initial
    std::vector<int> newOpen { tileId };

    int cout = 0;
    // Tant qu'il reste des noeuds � traiter ...
    while (!newOpen.empty()) {
        oldOpen = newOpen;
        newOpen = std::vector<int>();
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

int Npc::getId() const noexcept {
    return id;
}

int Npc::getTileId() const noexcept {
    return tileId;
}

int Npc::getTileObjectif() const noexcept {
    return tileObjectif;
}

void Npc::setTileObjectif(const int idTile) noexcept {
    tileObjectif = idTile;
}

Chemin& Npc::getChemin() noexcept {
    return chemin;
}

Distances& Npc::getEnsembleAccessible() noexcept {
    return ensembleAccessible;
}

bool Npc::isAccessibleTile(const int _tuileID) const noexcept {
    return std::find_if(begin(ensembleAccessible), end(ensembleAccessible), [&_tuileID](const DistanceType& type) {
        return type.tuileID == _tuileID;
    }) != end(ensembleAccessible);
}

int Npc::distanceToTile(const int _tuileID) {
    if (!isAccessibleTile(_tuileID))
        throw tile_inaccessible{};

    return std::find_if(begin(ensembleAccessible), end(ensembleAccessible), [&_tuileID](const DistanceType& type) {
        return type.tuileID == _tuileID;
    })->score;
}

bool Npc::isArrived() const noexcept {
    return estArrive;
}

void Npc::setArrived(const bool etat) noexcept {
    estArrive = etat;
}
