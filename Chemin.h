#ifndef CHEMIN_H
#define CHEMIN_H

#include <vector>
#include <string>

using std::vector;
using std::string;

class chemin_vide {}; // Exception levé lorsqu'une opération illégale est appelée sur un chemin vide

// Un chemin est constitué d'un vecteur contenant les indices des cases adjacentes sur lesquelles se déplacer !
class Chemin {
    bool inaccessible;
    vector<int> chemin;
public:

    Chemin();
    int distance() const noexcept;
    int destination() const noexcept;
    void setInaccessible() noexcept;
    bool isAccessible() const noexcept;
    bool empty() const noexcept;
    string toString() const noexcept;

    void removeFirst(); // Enlève la première case du chemin, utile pour mettre le chemin à jour
    void addFirst(const int); // Inverse de removeFirst, place une case supplémentaire au début du chemin
    void resetChemin() noexcept; // Réinitialise le chemin
    int getFirst() const; // Renvoie la première case du chemin
};



#endif
