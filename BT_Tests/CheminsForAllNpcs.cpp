#include "CheminsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"

#include "MyBotLogic/Tools/Minuteur.h"
#include "../GameManager.h"
#include "MyBotLogic/Tools/Profiler.h"

#include <sstream>
#include <vector>

using std::vector;
using std::stringstream;
using std::endl;

// Vérifier si un objectif est accessible pour tous nos NPCs
BT_Noeud::ETAT_ELEMENT CheminsForAllNpcs::execute() noexcept {
   ProfilerDebug profiler{ GameManager::getLogger(), "CheminsForAllNpcs::execute", GameManager::getLoggerJSON(), GameManager::getTempsDebutProgramme() };
   
   // Indices des objectfs découverts
   vector<unsigned int> objectifNonDonne = gm.c.getObjectifs();

   // On parcours chaque NPC
   for (auto& pair : gm.getNpcs()) {
      Npc& npc = pair.second;
      bool objFound = false;
      // On regarde si on pourra lui assigner un objectif
      vector<unsigned int>::iterator it = objectifNonDonne.begin();
	  while (!objFound && it != objectifNonDonne.end()) {
          if(npc.isAccessibleTile(*it)) {
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
         profiler << "Il n'y a pas de chemins pour tous les npcs !";
         // Si le cheminMin n'a pas été initialisé, c'est qu'il n'y a pas de chemins pour tous les npcs !
         return ETAT_ELEMENT::ECHEC;
      }
   }
   // Si tous les npc peuvent aller vers un objectif, on retourne REUSSI
   return ETAT_ELEMENT::REUSSI;
}