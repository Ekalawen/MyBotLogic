#include "Chemin.h"
#include "GameManager.h"

int Chemin::distance() {
    return chemin.size();
}

int Chemin::destination() {
    if (!chemin.empty()) {
        return chemin[0];
    } else {
        GameManager::Log("Attention, on essaye de récupérer la destination d'un chemin vide !");
        return -1;
    }
}

