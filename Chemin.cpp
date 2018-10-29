#include "Chemin.h"
#include "GameManager.h"

Chemin::Chemin()
	: chemin{ vector<int>{} },
    inaccessible{ false }
{
}

int Chemin::distance() const noexcept {
    if (!inaccessible)
        return static_cast<int>(chemin.size());
    return -1;
}

int Chemin::destination() const noexcept {
	if (!chemin.empty() && !inaccessible) {
		return chemin[0];
    } else {
        GameManager::Log("Attention, on essaye de récupérer la destination d'un chemin vide !");
        return -1;
    }
}

void Chemin::setInaccessible() noexcept {
    inaccessible = true;
}

bool Chemin::isAccessible() const noexcept {
    return !inaccessible;
}

bool Chemin::empty() const noexcept {
    return chemin.empty();
}

string Chemin::toString() const noexcept {
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


void Chemin::removeFirst() {
    if (empty())
        throw chemin_vide{};
    else
        chemin.pop_back();
}

void Chemin::addFirst(int elem) {
    chemin.push_back(elem);
}

int Chemin::getFirst() {
    if (empty())
        throw chemin_vide{};
    else
        return chemin.back();
}

void Chemin::resetChemin() {
    chemin.clear();
    inaccessible = false;
}
