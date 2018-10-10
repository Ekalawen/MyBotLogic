#include "Chemin.h"
#include "GameManager.h"

Chemin::Chemin()
    : chemin{ vector<int>() },
    inaccessible{false}
{
}

int Chemin::distance() {
    if (!inaccessible)
        return static_cast<int>(chemin.size());
    return -1;
}

int Chemin::destination() {
    if (!chemin.empty() && !inaccessible) {
        return chemin[0];
    } else {
        GameManager::Log("Attention, on essaye de récupérer la destination d'un chemin vide !");
        return -1;
    }
}

void Chemin::setInaccessible() {
    inaccessible = true;
}

bool Chemin::isAccessible() {
    return !inaccessible;
}

