#ifndef CHEMINS_FOR_ALL_NPCS_H
#define CHEMINS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CheminsForAllNpcs : public BT_Feuille {
   GameManager& manager;
public:
   CheminsForAllNpcs(GameManager& _manager) : manager{ _manager } {}


   // On vérifie si tous les npcs peuvent accéder à un objectif différent
   ETAT_ELEMENT execute() noexcept override;
};

#endif