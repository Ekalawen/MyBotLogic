#ifndef CHEMINS_FOR_ALL_NPCS_H
#define CHEMINS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CheminsForAllNpcs : public BT_Feuille {
   GameManager gm;
public:
   CheminsForAllNpcs(GameManager& gm) : gm{ gm } {}

   ETAT_ELEMENT execute() override {
       // Pr�condition v�rifi� : il y a au moins autant d'objectifs que de npcs
       // Calculer si il existe un chemin pour un objectif unique pour chacun des npcs

       // Pour cela on va calculer tous les chemins allant de tous les npcs � tous les objectifs
       // Et on va stocker cet ensemble de chemins, pour chaque npc, dans sa liste de cheminsPossibles
       for (auto& pair : gm.npcs) {
            // On r�cup�re le npc
            Npc& npc = pair.second;
            npc.resetChemins();

            // On r�cup�re tous les chemins � tous ses objectifs
            for (auto objectif : gm.m.objectifs) {
                Chemin chemin = gm.m.aStar(npc.tileId, objectif.second.id);
                npc.addChemin(chemin);
            }
       }

       // Ensuite on r�initialise la liste d'objectifsPris du GameManager
       gm.objectifPris = vector<int>{};

       // Puis en commancant par le Npc dont la plus courte distance � un objectif est la plus longue
       // On tente de lui affecte cet objectif et de le rajouter cet objectif dans objectifsPris
       // Tant qu'il reste des Npcs qui n'ont pas de chemins
       vector<int> npcAffectes;
       while (npcAffectes.size() < gm.npcs.size()) {
           // On r�cup�re le npc qui a le chemin minimal le plus long
           Npc* lastNpc;
           int distMax = -1;
           Chemin cheminMin;
           for (auto& pair_npc : gm.npcs) {
               Npc& npc = pair_npc.second;
               if (find(npcAffectes.begin(), npcAffectes.end(), npc.id) == npcAffectes.end()) { // Si on a pas d�j� affect� cet npc !
                   Chemin chemin = npc.getCheminMinNonPris(gm.objectifPris, gm.m.tailleCheminMax());
                   if (chemin.distance() > distMax) {
                       lastNpc = &npc;
                       distMax = chemin.distance();
                       cheminMin = chemin;
                   }
               }
           }

           // Puis on lui affecte son chemin le plus court si il y en a un !
           if(cheminMin.distance() <= 0) {
               // Si le cheminMin n'a pas �t� initialis�, c'est qu'il n'y a pas de chemins pour tous les npcs !
               GameManager::Log("Il n'y a pas de chemins pour tous les npcs !");
               return ETAT_ELEMENT::ECHEC;
           }
           lastNpc->chemin = cheminMin;
           lastNpc->tileObjectif = cheminMin.destination();
           gm.objectifPris.push_back(cheminMin.destination());

           npcAffectes.push_back(lastNpc->id);
       }

       // Si on a r�ussi � affecter un objectif � tous les Npcs, alors on renvoie un succ�s
       return ETAT_ELEMENT::REUSSI;
   }
};

#endif