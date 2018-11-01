#include "Npc.h"
#include "Globals.h"
#include "Voisin.h"
#include "GameManager.h"
#include"MyBotLogic/Tools/Minuteur.h"
#include <algorithm>
#include <sstream>

Npc::Npc(const NPCInfo _info) :
	id{ static_cast<int>(_info.npcID) },
	tileId{ static_cast<int>(_info.tileID) },
	tileObjectif{ -1 },
	chemin{ Chemin{} },
	estArrive{ false }
{
}

void Npc::move(const Tile::ETilePosition _direction, const Carte &_map) noexcept {
    tileId = _map.getAdjacentTileAt(tileId, _direction);
    _map.getTile(tileId).setStatut(MapTile::Statut::VISITE);
}

void Npc::resetChemins() noexcept {
    cheminsPossibles.clear();
    scoresAssocies.clear();
}

void Npc::addChemin(Chemin& _chemin) noexcept {
    cheminsPossibles.push_back(_chemin);
}

void Npc::addScore(ScoreType _score) noexcept {
    scoresAssocies.emplace_back(std::move(_score));
}

Chemin Npc::getCheminMinNonPris(const std::vector<int>& _objectifsPris, const int _tailleCheminMax) const noexcept {
    Chemin cheminMin;
    cheminMin.setInaccessible();
    int distMin = _tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); ++i) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas d�j� pris et qu'il est plus court !
		int destination = (chemin.empty()) ? tileId : chemin.destination(); // si le npc est d�j� arriv� il reste l�
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (_objectifsPris.empty() || std::find(_objectifsPris.begin(), _objectifsPris.end(), destination) == _objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;
}

int Npc::affecterMeilleurChemin(const Carte &_map) noexcept {
    stringstream ss;

    if (scoresAssocies.empty()) {
        // Dans ce cas-l� on reste sur place !
        chemin = Chemin{};
        ss << "Le Npc " << id << " n'a rien a rechercher et reste sur place !";
        GameManager::Log(ss.str());
        return tileId;
    }
	
    // On cherche le meilleur score
    auto preScore = Minuteur::now();
    auto bestIter = std::max_element(begin(scoresAssocies), end(scoresAssocies),
        [](const ScoreType& scoreDroite, const ScoreType& scoreGauche){
            return scoreDroite.score < scoreGauche.score;
        });
    ss << "Dur�e chercher meilleur score = " << std::chrono::duration_cast<std::chrono::microseconds>(postScore - preScore).count() / 1000.f << "ms" <<std::endl;
    auto postScore = Minuteur::now();

    // On affecte son chemin, mais il nous faut le calculer ! =)
    auto preAStar = Minuteur::now();
    chemin = _map.aStar(tileId, bestIter->tuileID);
    auto postAStar = Minuteur::now();

    ss << "Le Npc " << to_string(id) << " va rechercher la tile " << chemin.destination() << std::endl;
    ss << "Dur�e a* = " << std::chrono::duration_cast<std::chrono::microseconds>(postAStar - preAStar).count() / 1000.f << "ms" << std::endl;
    GameManager::Log(ss.str());

    // On renvoie la destination
    return chemin.destination();
}

void Npc::floodfill(const Map &_map) {
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
            for (auto voisinID : _map.getTile(tileID).getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
                // Si elle est connu
                if (_map.getTile(voisinID).existe()) {
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

void Npc::setTileObjectif(const int _idTile) noexcept {
    tileObjectif = _idTile;
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

void Npc::setArrived(const bool _etat) noexcept {
    estArrive = _etat;
}
