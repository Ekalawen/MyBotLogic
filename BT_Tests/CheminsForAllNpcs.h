#ifndef CHEMINS_FOR_ALL_NPCS_H
#define CHEMINS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CheminsForAllNpcs : public BT_Feuille {

   GameManager gm;
public:
   CheminsForAllNpcs(GameManager& gm) : gm{ gm } {}


   ETAT_ELEMENT execute() override {
      // Précondition vérifié : il y a au moins autant d'objectifs que de npcs
      // Calculer si il existe un chemin pour un objectif unique pour chacun des npcs
      return ETAT_ELEMENT::ECHEC;
   }
};

#endif