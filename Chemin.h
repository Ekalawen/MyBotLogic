#ifndef CHEMIN_H
#define CHEMIN_H

#include <vector>
using namespace std;

// Un chemin est constitué d'un vecteur contenant les indices des cases adjacentes sur lesquelles se déplacer !
class Chemin {
    bool inaccessible;
public:
    vector<int> chemin;

    Chemin();
    int distance() noexcept;
    int destination() noexcept;
    void setInaccessible() noexcept;
    bool isAccessible() noexcept;
    bool empty() noexcept;
    string toString() noexcept;
};



#endif
