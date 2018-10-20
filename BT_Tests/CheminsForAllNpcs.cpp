#include "CheminsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"
#include <chrono>

// Vérifier si un objectif est accessible pour tous nos NPCs
BT_Noeud::ETAT_ELEMENT CheminsForAllNpcs::execute() noexcept {
   auto pre = std::chrono::high_resolution_clock::now();
   GameManager::Log("CheminsForAllNpcs");
   
   // Indices des objectfs découverts
   vector<unsigned int> objectifNonDonne;
   for (auto pair : gm.m.objectifs) {
      objectifNonDonne.push_back(pair.first);
   }

   // On parcours chaque NPC
   for (auto& pair : gm.npcs) {
      Npc& npc = pair.second;
      bool objFound = false;
      int indPosObj = 0;
      // On regarde si on pourra lui assigner un objectif
      for (unsigned int objectifID : objectifNonDonne) {
         if (find(npc.ensembleAccessible.begin(), npc.ensembleAccessible.end(), objectifID) != npc.ensembleAccessible.end()) {
            objFound = true;
            break;
         }
         ++indPosObj;
      }
      // Si c'est le cas, on enlève cet objectif de ceux attribuables et on continue
      if (objFound) {
         objectifNonDonne.erase(objectifNonDonne.begin() + indPosObj);
      // Sinon on retourne ECHEC
      } else {
         auto post = std::chrono::high_resolution_clock::now();
         GameManager::Log("Durée CheminsForAll = " + to_string(std::chrono::duration_cast<std::chrono::microseconds>(post - pre).count() / 1000.f) + "ms");
         // Si le cheminMin n'a pas été initialisé, c'est qu'il n'y a pas de chemins pour tous les npcs !
         GameManager::Log("Il n'y a pas de chemins pour tous les npcs !");
         return ETAT_ELEMENT::ECHEC;
      }
      // Si tous les npc peuvent aller vers un objectif, on retourne REUSSI
      return ETAT_ELEMENT::REUSSI;
   }
}