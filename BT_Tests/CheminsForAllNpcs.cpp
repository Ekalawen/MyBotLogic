#include "CheminsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"
#include "MyBotLogic/Tools/Minuteur.h"

#include <sstream>
#include <vector>

using std::vector;
using std::stringstream;
using std::endl;

// V�rifier si un objectif est accessible pour tous nos NPCs
BT_Noeud::ETAT_ELEMENT CheminsForAllNpcs::execute() noexcept {
   auto pre = Minuteur::now();
   GameManager::log("CheminsForAllNpcs");
   
   // Indices des objectfs d�couverts
   vector<unsigned int> objectifNonDonne = gm.carte.getObjectifs();

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

      // Si c'est le cas, on enl�ve cet objectif de ceux attribuables et on continue
      if (objFound) {
         objectifNonDonne.erase(it);
      // Sinon on retourne ECHEC
      } else {
         auto post = Minuteur::now();
         stringstream ss;
         ss << "Dur�e CheminsForAll = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl
            << "Il n'y a pas de chemins pour tous les npcs !";
         GameManager::log(ss.str());
         // Si le cheminMin n'a pas �t� initialis�, c'est qu'il n'y a pas de chemins pour tous les npcs !
         return ETAT_ELEMENT::ECHEC;
      }
   }
   // Si tous les npc peuvent aller vers un objectif, on retourne REUSSI
   return ETAT_ELEMENT::REUSSI;
}