#ifndef CONTRAINTE_H
#define CONTRAINTE_H

#include "Globals.h"
#include <vector>
#include <set>
using std::vector;



// Une contrainte repr�sente une porte sur le chemin d'un Npc
// Il faudra qu'un autre Npc se d�place � la case du switch pour activer cette contrainte !
// Une contrainte n'existe que relativement � un chemin !
class Carte;
class GameManager;
class Chemin;
class Contrainte {
private:
    int caseAvantPorte;
    int caseApresPorte;
    Tile::ETilePosition direction;
    std::set<int> casesDesSwitchs;
    int npcAssocie = -1;
    Chemin* cheminAssocie = nullptr; // �a ne peut pas �tre un objet sinon on aurait une boucle infinie x)

public:
    Contrainte() = default;
    ~Contrainte();
    Contrainte(const int caseAvantPorte, const int caseApresPorte, const std::set<int> casesDesSwitchs, const Carte& c);
    bool estNonContraignante() const noexcept; // Permet de savoir si la case du switch est sur la case avant la porte, si c'est le cas, la contrainte n'est pas vraiment g�nante !
    bool isSolvableWithout(const vector<int> npcsIds, GameManager& gm, int& nbToursAvantOuvertur) const noexcept; // Permet de savoir si la contrainte est atteignable par un autre npc que celui qui a besoin de la contrainte :)

    int getCaseAvantPorte() const noexcept;
    int getCaseApresPorte() const noexcept;

    void resoudre(int npc, Chemin& chemin);
    void setNpc(int npc);
    int tempsResolution(); // � n'appeller que si la contrainte est d�j� r�solue !
    void deResoudre();
    bool estResolue();
    int getNpcAssocie();
    Chemin* getCheminAssocie();

    static int prixContraintes(const vector<Contrainte>& contraintes);
    static int prixContrainte(Contrainte& contrainte);
};


#endif



