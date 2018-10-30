#include "ObjectifsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT ObjectifsForAllNpcs::execute() noexcept {
    GameManager::log("ObjectifsForAllNpcs");
    if (manager.map.getObjectifs().size() >= manager.getNpcs().size()) {
        return ETAT_ELEMENT::REUSSI;
    } else {
        return ETAT_ELEMENT::ECHEC;
    }
}
