#include "Npc.h"
#include "Globals.h"
#include "GameManager.h"
#include <chrono>

Npc::Npc(const NPCInfo _info) :
	id{ static_cast<int>(_info.npcID) },
	tileId{ static_cast<int>(_info.tileID) },
	tileObjectif{ -1 },
	chemin{ Chemin{} },
	estArrive{ false }
{
}


void Npc::move(Tile::ETilePosition _direction, Carte &_map) noexcept {
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

void Npc::addScore(int _tileIndice, float _score) noexcept {
    scoresAssocies[_tileIndice] = _score;
}

Chemin Npc::getCheminMinNonPris(std::vector<int> _objectifsPris, int _tailleCheminMax) const noexcept {
    Chemin cheminMin;
    cheminMin.setInaccessible();
    int distMin = _tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); ++i) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas déjà pris et qu'il est plus court !
		int destination = (chemin.empty()) ? tileId : chemin.destination(); // si le npc est déjà arrivé il reste là
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (_objectifsPris.empty() || std::find(_objectifsPris.begin(), _objectifsPris.end(), destination) == _objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;
}

void testBestScore(std::pair<const int, float> _pair, float& _bestScore, int& _bestScoreIndice) {
    int tileId = _pair.first;
    float score = _pair.second;
    GameManager::log("Case potentielle à explorer : " + std::to_string(tileId) + " de score " + std::to_string(score));
    if (score > _bestScore) {
        _bestScore = score;
        _bestScoreIndice = tileId;
    }
}

int Npc::affecterMeilleurChemin(Carte &_map) noexcept {
    if (scoresAssocies.empty()) {
        // Dans ce cas-là on reste sur place !
        chemin = Chemin{};
        GameManager::log("Le Npc " + std::to_string(id) + " n'a rien a rechercher et reste sur place !");
        return tileId;
    }
	
    // On cherche le meilleur score
    auto preScore = std::chrono::high_resolution_clock::now();
    float bestScore = scoresAssocies.begin()->second;
    int bestScoreIndice = scoresAssocies.begin()->first;
    for (auto pair : scoresAssocies) {
        testBestScore(pair, bestScore, bestScoreIndice);
    }
    auto postScore = std::chrono::high_resolution_clock::now();
    GameManager::log("Durée chercher meilleur score = " + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(postScore - preScore).count() / 1000.f) + "ms");

    // On affecte son chemin, mais il nous faut le calculer ! =)
    auto preAStar = std::chrono::high_resolution_clock::now();
    chemin = _map.aStar(tileId, bestScoreIndice);
    auto postAStar = std::chrono::high_resolution_clock::now();
    GameManager::log("Le Npc " + std::to_string(id) + " va rechercher la tile " + std::to_string(chemin.destination()));
    GameManager::log("Durée a* = " + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(postAStar - preAStar).count() / 1000.f) + "ms");

    // On renvoie la destination
    return chemin.destination();
}

void ajoutIfUnkown(Carte &_map, int _voisin, const std::vector<int>& _oldOpen, const std::vector<int>& _open, std::vector<int>& _newOpen) {
    // Si elle est connu
    if (_map.getTile(_voisin).existe()) {
       // Si elle n'est pas déjà ajouté
       if (find(_oldOpen.begin(), _oldOpen.end(), _voisin) == _oldOpen.end() && find(_open.begin(), _open.end(), _voisin) == _open.end()) {
          // On l'ajoute comme nouvelle tuile ouverte
          _newOpen.push_back(_voisin);
       }
    }
}

void addNewVoisins(Carte &_map, int _tileID, const std::vector<int>& _oldOpen, std::vector<int>& _open, std::vector<int>& _newOpen, std::map<int, int>& _coutCasesAccessibles, int _cout) {
     for (auto voisin : _map.getTile(_tileID).getVoisinsAccessibles()) {
         ajoutIfUnkown(_map, voisin, _oldOpen, _open, _newOpen);
     }
     // On définit les dernières tuiles ajoutés avec leur coût courant
     if (find(_open.begin(), _open.end(), _tileID) == _open.end()) {
        _open.push_back(_tileID);
        _coutCasesAccessibles[_tileID] = _cout;
     }       
}

void parcourirNewVoisins(Carte &_map, int _tileID, std::vector<int>& _oldOpen, std::vector<int>& _open, std::vector<int>& _newOpen, std::map<int, int>& _coutCasesAccessibles, int& _cout) {
    _oldOpen = _newOpen;
    _newOpen = std::vector<int>();
    // On regarde les voisins des dernieres tuiles ajoutées
    for (int tileID : _oldOpen) {
        addNewVoisins(_map, tileID, _oldOpen, _open, _newOpen, _coutCasesAccessibles, _cout);
    }
    _cout++;
}

void Npc::floodfill(Carte &_map) {
   std::vector<int> _open;
   std::vector<int> _oldOpen;
   std::vector<int> _newOpen;
   std::map<int, int> _coutCasesAccessibles;

   // On ajoute le noeud initial
   _newOpen.push_back(tileId);

   int cout = 0;
   // Tant qu'il reste des noeuds à traiter ...
   while (!_newOpen.empty()) {
       parcourirNewVoisins(_map, tileId, _oldOpen, _open, _newOpen, _coutCasesAccessibles, cout);
   }

   // On met à jour l'ensemble et les distances accessible d'un NPC
   ensembleAccessible = _open;
   distancesEnsembleAccessible = _coutCasesAccessibles;
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

void Npc::setTileObjectif(int _idTile) {
    tileObjectif = _idTile;
}

Chemin& Npc::getChemin() {
    return chemin;
}

std::vector<int> Npc::getEnsembleAccessible() {
    return ensembleAccessible;
}

bool Npc::isAccessibleTile(int _tileId) {
    return find(ensembleAccessible.begin(), ensembleAccessible.end(), _tileId) != ensembleAccessible.end();
}

int Npc::distanceToTile(int _tileId) {
    if (!isAccessibleTile(_tileId))
        throw tile_inaccessible{};
    return distancesEnsembleAccessible[_tileId];
}

std::map<int, int> Npc::getDistancesEnsembleAccessible() {
    return distancesEnsembleAccessible;
}

bool Npc::isArrived() {
    return estArrive;
}

void Npc::setArrived(bool _etat) {
    estArrive = _etat;
}
