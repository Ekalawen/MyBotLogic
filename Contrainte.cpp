#include "Contrainte.h"
#include "Carte.h"
#include "GameManager.h"
#include "Chemin.h"
#include <set>

Contrainte::Contrainte(const int caseAvantPorte, const int caseApresPorte, const std::set<int> casesDesSwitchs, const Carte& c)
    : caseAvantPorte{ caseAvantPorte }, caseApresPorte{ caseApresPorte }, casesDesSwitchs{ casesDesSwitchs }, direction{ c.getDirection(caseAvantPorte, caseApresPorte) }
{
}

Contrainte::~Contrainte() {
    //delete cheminAssocie;
}

bool Contrainte::estNonContraignante() const noexcept {
    for (int caseSwitch : casesDesSwitchs) {
        if (caseSwitch == caseAvantPorte)
            return true;
    }
    return false;
}

bool Contrainte::isSolvableWithout(const vector<int> npcsOccupesIds, GameManager& gm, int& nbToursAvantOuverture) const noexcept {
    // Déjà on vérifie que notre contrainte est vraiment contraignante !
    if (estNonContraignante())
        return true;
    else {
        // Si la contrainte est contraingante, alors il faut voir si un autre Npc peut y accéder à notre place !

        for (auto& npc_pair : gm.getNpcs()) {
            Npc& npcSuppleant = npc_pair.second;
            // On vérifie que le Npc n'est pas déjà occupé
            if (find(npcsOccupesIds.begin(), npcsOccupesIds.end(), npcSuppleant.getId()) == npcsOccupesIds.end()) {
                // On vérifie qu'il peut atteindre au moins l'un des interupteurs
                for (int switchTileId : casesDesSwitchs) {
                    vector<int> newNpcsOccupesIds = npcsOccupesIds; newNpcsOccupesIds.push_back(npcSuppleant.getId()); // Not used in A* for the moment !
                    Chemin chemin = gm.c.aStar(npcSuppleant.getTileId(), switchTileId, npcSuppleant.getId(), gm);
                    if (chemin.isAccessible()) {
                        nbToursAvantOuverture = chemin.distance();
                        return true;
                    }
                }
            }
        }

        return false;
    }
}

int Contrainte::getCaseAvantPorte() const noexcept {
    return caseAvantPorte;
}
int Contrainte::getCaseApresPorte() const noexcept {
    return caseApresPorte;
}

void Contrainte::resoudre(int npc, Chemin& chemin) {
    npcAssocie = npc;
    cheminAssocie = new Chemin(chemin);
}

void Contrainte::setNpc(int npc) {
    npcAssocie = npc;
}

void Contrainte::deResoudre() {
    npcAssocie = -1;
    delete cheminAssocie;
    cheminAssocie = nullptr;
}
bool Contrainte::estResolue() {
    return (npcAssocie != -1) && (cheminAssocie != nullptr);
}

int Contrainte::getNpcAssocie() {
    return npcAssocie;
}
Chemin* Contrainte::getCheminAssocie() {
    return cheminAssocie;
}

int Contrainte::tempsResolution() {
    return cheminAssocie->distance();
}

int Contrainte::prixContrainte(Contrainte& contrainte) {
    int prix = 0;
    if (contrainte.estResolue()) {
        prix += contrainte.getCheminAssocie()->distanceBrute();
        prix += Contrainte::prixContraintes(contrainte.getCheminAssocie()->getContraintes());
    }
    return prix;
}

int Contrainte::prixContraintes(const vector<Contrainte>& contraintes) {
    int prix = 0;
    for (Contrainte contrainte : contraintes) {
        prix += Contrainte::prixContrainte(contrainte);
    }
    return prix;
}
