#ifndef CHEMIN_H
#define CHEMIN_H

#include "Contrainte.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class chemin_vide {}; // Exception lev� lorsqu'une op�ration ill�gale est appel�e sur un chemin vide

// Un chemin est constitu� d'un vecteur contenant les indices des cases adjacentes sur lesquelles se d�placer !
class Npc;
class GameManager;
class Chemin {
    bool inaccessible;
    vector<int> chemin;
    vector<Contrainte> contraintes;

public:

    Chemin();
    int distance() const noexcept;
    int distanceBrute() const noexcept; // La distance sans prendre en compte les contraintes. N'utilisez-l� que si vous savez ce que vous faites !
    int destination() const noexcept;
    void setInaccessible() noexcept;
    bool isAccessible() const noexcept;
    bool empty() const noexcept;
    string toString() const noexcept;

    void removeFirst(); // Enl�ve la premi�re case du chemin, utile pour mettre le chemin � jour
    void addFirst(const int); // Inverse de removeFirst, place une case suppl�mentaire au d�but du chemin
    void resetChemin() noexcept; // R�initialise le chemin
    int getFirst() const; // Renvoie la premi�re case du chemin

    void addContrainte(const Contrainte& contrainte);
    void setContraintes(const vector<Contrainte> contraintes);
    void resetContraintes();
    void cleanContraintes(); // Supprime toutes les contraintes qui ne sont pas r�solues
    vector<Contrainte>& getContraintes();
    vector<int> affecterContraintes(int npcAffecte, GameManager& gm);
};



#endif
