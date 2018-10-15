#include "Chemin.h"
#include "GameManager.h"

Chemin::Chemin()
    : chemin{ vector<int>() },
    inaccessible{ false }
{
}

int Chemin::distance() noexcept {
    if (!inaccessible)
        return static_cast<int>(chemin.size());
    return -1;
}

int Chemin::destination() noexcept {
    if (!chemin.empty() && !inaccessible) {
        return chemin[0];
    } else {
        GameManager::Log("Attention, on essaye de r�cup�rer la destination d'un chemin vide !");
        return -1;
    }
}

void Chemin::setInaccessible() noexcept {
    inaccessible = true;
}

bool Chemin::isAccessible() noexcept {
    return !inaccessible;
}

bool Chemin::empty() noexcept {
    return chemin.empty();
}

string Chemin::toString() noexcept {
    string str;
    if (chemin.size() <= 0) {
        str += "vide";
    } else {
        if (!isAccessible()) {
            str += "inaccessible : ";
        }
        for (int i = static_cast<int>(chemin.size() - 1); i >= 0; i--) {
            str += to_string(chemin[i]) + " ";
        }
        str.pop_back();
    }
    return str;
}

