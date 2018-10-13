#ifndef CHEMINS_FOR_ALL_NPCS_H
#define CHEMINS_FOR_ALL_NPCS_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"
#include "../GameManager.h"

class CheminsForAllNpcs : public BT_Feuille {
   GameManager& gm;
   bool aDejaReussi;
public:
    CheminsForAllNpcs(GameManager& gm) : gm{ gm }, aDejaReussi{ false } {}

    ETAT_ELEMENT execute() override;
};

#endif