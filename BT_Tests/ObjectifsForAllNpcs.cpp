#include "ObjectifsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"
#include "../GameManager.h"

BT_Noeud::ETAT_ELEMENT ObjectifsForAllNpcs::execute() noexcept {
   LOG("ObjectifsForAllNpcs");
    if (manager.c.getObjectifs().size() >= manager.getNpcs().size()) {
        return ETAT_ELEMENT::REUSSI;
    } else {
        return ETAT_ELEMENT::ECHEC;
    }
}
