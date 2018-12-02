#ifndef OBJECTIFS_FOR_ALL_NPCS_H
#define OBJECTIFS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"

class GameManager;
class ObjectifsForAllNpcs : public BT_Feuille {
   GameManager& gm;
public:
   ObjectifsForAllNpcs(GameManager& gm) : gm{ gm } {}

   // M�thode naive car si on voit un objectif, on consid�re qu'il est atteignable !!!
   // On devrait v�rifier si il y a au moins autant d'objectifs potentiellement atteignables que de npcs !!!
   ETAT_ELEMENT execute() noexcept override;
};

#endif