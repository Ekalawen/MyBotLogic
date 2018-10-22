
#include "Npc.h"
#include "Globals.h"
#include "GameManager.h"
#include <chrono>

Npc::Npc(const NPCInfo info) :
	id{ static_cast<int>(info.npcID) },
	tileId{ static_cast<int>(info.tileID) },
	tileObjectif{ -1 },
	chemin{},
	estArrive{ false }
{
}


void Npc::move(Tile::ETilePosition direction, Map &m) noexcept {
    tileId = m.getAdjacentTileAt(tileId, direction);
    m.tiles[tileId].statut = MapTile::Statut::VISITE;
}

void Npc::resetChemins() noexcept {
    cheminsPossibles.clear();
    scoresAssocies.clear();
}

void Npc::addChemin(Chemin& chemin) noexcept {
    cheminsPossibles.push_back(chemin);
}

void Npc::addScore(int tileIndice, float score) noexcept {
    scoresAssocies[tileIndice] = score;
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

int Npc::affecterMeilleurChemin(Map m) noexcept {
    if (scoresAssocies.empty()) {
        // Dans ce cas-l� on reste sur place !
        chemin = Chemin{};
        GameManager::Log("Le Npc " + to_string(id) + " n'a rien a rechercher et reste sur place !");
        return tileId;
    }
	
    // On cherche le meilleur score
    float bestScore = scoresAssocies.begin()->second;
    int bestScoreIndice = scoresAssocies.begin()->first;
    for (auto pair : scoresAssocies) {
        int tileId = pair.first;
        float score = pair.second;
        GameManager::Log("Case potentielle � explorer : " + to_string(tileId) + " de score " + to_string(score));
        if (score > bestScore) {
            bestScore = score;
            bestScoreIndice = tileId;
        }
    }

    // On affecte son chemin, mais il nous faut le calculer ! =)
    chemin = m.aStar(tileId, bestScoreIndice);
    GameManager::Log("Le Npc " + to_string(id) + " va rechercher la tile " + to_string(chemin.destination()));

    // On renvoie la destination
    return chemin.destination();
}

void Npc::floodfill(Map &m) {
   auto pre = std::chrono::high_resolution_clock::now();
   vector<int> Open;
   vector<int> oldOpen;
   vector<int> newOpen;
   map<int, int> coutCasesAccessibles;


   // On ajoute le noeud initial
   newOpen.push_back(tileId);

   int cout = 0;
   // Tant qu'il reste des noeuds � traiter ...
   while (!newOpen.empty()) {
      oldOpen = newOpen;
      newOpen = vector<int>();
      // On regarde les voisins des dernieres tuiles ajout�es
      for (int tileID : oldOpen) {
         for (auto voisin : m.tiles[tileID].voisinsAccessibles) {
            // Si elle est connu
            if (m.tiles[voisin].existe()) {
               // Si elle n'est pas d�j� ajout�
               if (find(oldOpen.begin(), oldOpen.end(), voisin) == oldOpen.end() && find(Open.begin(), Open.end(), voisin) == Open.end()) {
                  // On l'ajoute comme nouvelle tuile ouverte
                  newOpen.push_back(voisin);
               }
            }
         }
         // On d�finit les derni�res tuiles ajout�s avec leur co�t courant
         if (find(Open.begin(), Open.end(), tileID) == Open.end()) {
            Open.push_back(tileID);
            coutCasesAccessibles[tileID] = cout;
         }       
      }
      ++cout;
   }
   // On met � jour l'ensemble et les distances accessible d'un NPC
   ensembleAccessible = Open;
   distancesEnsembleAccessible = coutCasesAccessibles;
   auto post = std::chrono::high_resolution_clock::now();
   GameManager::Log("Dur�e FloodFill = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");

}
