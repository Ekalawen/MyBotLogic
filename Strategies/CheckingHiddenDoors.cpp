
#include "CheckingHiddenDoors.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/GameManager.h"
#include "MyBotLogic/MapTile.h"

#include <string>
#include <vector>
using std::vector;

BT_Noeud::ETAT_ELEMENT CheckingHiddenDoors::execute() noexcept {
    ProfilerRelease* pointeurProfiler = new ProfilerRelease( GameManager::getLoggerRelease(), "CheckingHiddenDoors", true, true );
    ProfilerDebug profiler{ GameManager::getLogger(), string("CheckingHiddenDoors") };
    LOG("HEY !");

    // Pour tous les npcs
    for (auto& pair : manager.getNpcs()) {
        Npc& npc = pair.second;

        // On regarde s'ils sont à coté d'une mur à checker
        if (isAdajacentToWallToCheck(npc.getTileId())) {
            // Si oui, il check
            affecterDoorACheckerAuNpc(npc);

        } else {
            // Si non, on lui affecte un chemin vers un mur à checker
            affecterCheminNpcPourCheckerDoors(npc);
        }
    }

    return ETAT_ELEMENT::REUSSI;
}

void CheckingHiddenDoors::affecterDoorACheckerAuNpc(Npc& npc) {
    for (Voisin voisin : manager.c.getTile(npc.getTileId()).getVoisins()) {
        int voisinID = voisin.getTuileIndex();
        if (manager.c.hasWallBetweenUnchecked(npc.getTileId(), voisinID)) {
            npc.setIsCheckingDoor(true, manager.c.getDirection(npc.getTileId(), voisinID));
            return;
        }
    }
    LOG("On ne devrait pas être ici !!!");
}

bool CheckingHiddenDoors::isAdajacentToWallToCheck(const int npcTileID) const noexcept {
    for (Voisin voisin : manager.c.getTile(npcTileID).getVoisins()) {
        int voisinID = voisin.getTuileIndex();
        if (manager.c.hasWallBetweenUnchecked(npcTileID, voisinID))
            return true;
    }
    return false;
}

void CheckingHiddenDoors::affecterCheminNpcPourCheckerDoors(Npc& npc) {
    npc.resetObjectifs();

    // Calculer le score de chaque tile pour le npc
    // En même temps on calcul le chemin pour aller à cette tile
    // On stocke ces deux informations dans l'attribut cheminsPossibles du Npc
    calculerScoresTilesPourNpc(npc);

    // Choisir la meilleure tile pour ce npc et lui affecter son chemin
    int tileChoisi = npc.affecterMeilleurObjectif(manager);
}

void CheckingHiddenDoors::calculerScoresTilesPourNpc(Npc& _npc) noexcept {
   ProfilerDebug profiler{ GameManager::getLogger(), "calculerScoresTilesPourNpc", false};
   //profiler << "Taille ensemble : " << _npc.getEnsembleAccessible().size() << endl;
   for (auto score : _npc.getEnsembleAccessible()) { // parcours toutes les tiles découvertes par l'ensemble des npcs et qui sont accessibles
      calculerScore1Tile(score.tuileID, manager.c, _npc);
   }
}

void CheckingHiddenDoors::calculerScore1Tile(int _tileID, Carte& _carte, Npc& _npc) {
   MapTile tile = _carte.getTile(_tileID);
   saveScore(tile, _npc);
}

void CheckingHiddenDoors::saveScore(const MapTile& _tile, Npc& _npc) const noexcept {
    float score = 0;

    // On enregistre le cout, cad la distanc npc-tile
    score += _npc.distanceToTile(_tile.getId()) * COEF_DISTANCE_NPC_TILE;

    // On regarde l'intéret de cette tile
    float interetTile = interet(_tile);
    score += interetTile * COEF_INTERET;
    if (interetTile == 0) return; // Si pas d'intéret, la tile ne nous intéresse pas !

    // Il reste à affecter le score et le chemin au npc
    _npc.addScore({ _tile.getId(), score });
}

// L'intérét est définit par :
    // Le nombre de murs que possède la tuile
float CheckingHiddenDoors::interet(const MapTile& tile) const noexcept {
    float interet = 0;

    // On compte le nombre de murs
    int nbMursAdjacents = 0;
    for (Voisin voisin : tile.getVoisins()) {
        int voisinID = voisin.getTuileIndex();
        if (manager.c.hasWallBetweenUnchecked(tile.getId(), voisinID))
            nbMursAdjacents++;
    }

    interet += nbMursAdjacents * COEF_NB_MURS_TILE;;

    return interet;
}
