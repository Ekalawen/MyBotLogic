#ifndef OBJECTIFS_FOR_ALL_NPCS_H
#define OBJECTIFS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class ObjectifsForAllNpcs : public BT_Feuille {
   GameManager& manager;
public:
   ObjectifsForAllNpcs(GameManager& _manager) : manager{ _manager } {}

   // M�thode naive car si on voit un objectif, on consid�re qu'il est atteignable !!!
   // On devrait v�rifier si il y a au moins autant d'objectifs potentiellement atteignables que de npcs !!!
   ETAT_ELEMENT execute() noexcept override;
};

#endif