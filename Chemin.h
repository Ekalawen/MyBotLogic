#ifndef CHEMIN_H
#define CHEMIN_H

#include "Contrainte.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class chemin_vide {}; // Exception levé lorsqu'une opération illégale est appelée sur un chemin vide

// Un chemin est constitué d'un vecteur contenant les indices des cases adjacentes sur lesquelles se déplacer !
class Npc;
class GameManager;
class Chemin {
    bool inaccessible;
    vector<int> chemin;
    vector<Contrainte> contraintes;

public:

    Chemin();
    int distance() const noexcept;
    int distanceBrute() const noexcept; // La distance sans prendre en compte les contraintes. N'utilisez-là que si vous savez ce que vous faites !
    int destination() const noexcept;
    void setInaccessible() noexcept;
    bool isAccessible() const noexcept;
    bool empty() const noexcept;
    string toString() const noexcept;

    void removeFirst(); // Enlève la première case du chemin, utile pour mettre le chemin à jour
    void addFirst(const int); // Inverse de removeFirst, place une case supplémentaire au début du chemin
    void resetChemin() noexcept; // Réinitialise le chemin
    int getFirst() const; // Renvoie la première case du chemin

    void addContrainte(const Contrainte& contrainte);
    void setContraintes(const vector<Contrainte> contraintes);
    void resetContraintes();
    void cleanContraintes(); // Supprime toutes les contraintes qui ne sont pas résolues
    vector<Contrainte>& getContraintes();
    vector<int> affecterContraintes(int npcAffecte, GameManager& gm);
};



#endif
