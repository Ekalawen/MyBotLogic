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
    int distance();
    int destination();
    void setInaccessible();
    bool isAccessible();
};



#endif
