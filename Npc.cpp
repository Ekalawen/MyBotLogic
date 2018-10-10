
#include "Npc.h"
#include "Globals.h"

Npc::Npc(const NPCInfo info) : 
    id{ static_cast<int>(info.npcID) },
    tileId{static_cast<int>(info.tileID)},
    tileObjectif{ -1 },
    chemin{}
{
}


void Npc::move(Tile::ETilePosition direction, Map m) {
    tileId = m.getAdjacentTileAt(tileId, direction);
}

void Npc::resetChemins() {
    cheminsPossibles.clear();
}

void Npc::addChemin(Chemin& chemin) {
    cheminsPossibles.push_back(chemin);
}

Chemin Npc::getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) {
    Chemin cheminMin;
    int distMin = tailleCheminMax;

    for (int i = 0; i < cheminsPossibles.size(); i++) {
        Chemin chemin = cheminsPossibles[i];
        // Si le chemin n'est pas déjà pris et qu'il est plus court !
        if (chemin.isAccessible()
        && chemin.distance() < distMin
        && (objectifsPris.empty() || find(objectifsPris.begin(), objectifsPris.end(), chemin.destination()) == objectifsPris.end())) {
            cheminMin = chemin;
            distMin = chemin.distance();
        }
    }

    return cheminMin;
}
