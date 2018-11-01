#ifndef CHEMINS_FOR_ALL_NPCS_H
#define CHEMINS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CheminsForAllNpcs : public BT_Feuille {
   GameManager& gm;
public:
   CheminsForAllNpcs(GameManager& gm) : gm{ gm } {}


   // On v�rifie si tous les npcs peuvent acc�der � un objectif diff�rent
   ETAT_ELEMENT execute() noexcept override;
};

#endif