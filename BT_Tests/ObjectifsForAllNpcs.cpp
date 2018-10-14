#include "ObjectifsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT ObjectifsForAllNpcs::execute() {
    GameManager::Log("ObjectifsForAllNpcs");
    if (gm.m.objectifs.size() >= gm.npcs.size()) {
        return ETAT_ELEMENT::REUSSI;
    } else {
        return ETAT_ELEMENT::ECHEC;
    }
}
