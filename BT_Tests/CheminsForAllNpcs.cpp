#include "CheminsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

// Vérifier si un objectif est accessible pour tous nos NPCs
BT_Noeud::ETAT_ELEMENT CheminsForAllNpcs::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();
   GameManager::Log("CheminsForAllNpcs");
   
   // Indices des objectfs découverts
   vector<unsigned int> objectifNonDonne = gm.m.objectifs;

   // On parcours chaque NPC
   for (auto& pair : gm.npcs) {
      Npc& npc = pair.second;
      bool objFound = false;
      // On regarde si on pourra lui assigner un objectif
	  vector<unsigned int>::iterator it = objectifNonDonne.begin();
	  while (!objFound && it != objectifNonDonne.end()) {
		  if (find(npc.ensembleAccessible.begin(), npc.ensembleAccessible.end(), (*it)) != npc.ensembleAccessible.end()) {
			  objFound = true;
		  }
		  else {
			  ++it;
		  }
	  }

      // Si c'est le cas, on enlève cet objectif de ceux attribuables et on continue
      if (objFound) {
         objectifNonDonne.erase(it);
      // Sinon on retourne ECHEC
      } else {
         auto post = std::chrono::high_resolution_clock::now();
         GameManager::Log("Durée CheminsForAll = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
         // Si le cheminMin n'a pas été initialisé, c'est qu'il n'y a pas de chemins pour tous les npcs !
         GameManager::Log("Il n'y a pas de chemins pour tous les npcs !");
         return ETAT_ELEMENT::ECHEC;
      }
   }
   // Si tous les npc peuvent aller vers un objectif, on retourne REUSSI
   return ETAT_ELEMENT::REUSSI;
}