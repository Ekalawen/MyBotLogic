#ifndef CHEMIN_H
#define CHEMIN_H

#include <vector>
#include <string>

using std::vector;
using std::string;

class chemin_vide {}; // Exception lev� lorsqu'une op�ration ill�gale est appel�e sur un chemin vide

// Un chemin est constitu� d'un vecteur contenant les indices des cases adjacentes sur lesquelles se d�placer !
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

    void removeFirst(); // Enl�ve la premi�re case du chemin, utile pour mettre le chemin � jour
    void addFirst(const int); // Inverse de removeFirst, place une case suppl�mentaire au d�but du chemin
    void resetChemin() noexcept; // R�initialise le chemin
    int getFirst() const; // Renvoie la premi�re case du chemin
};



#endif
