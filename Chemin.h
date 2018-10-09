#ifndef CHEMIN_H
#define CHEMIN_H

#include <vector>
using namespace std;

// Un chemin est constitué d'un vecteur contenant les indices des cases adjacentes sur lesquelles se déplacer !
class Chemin {
public:
    vector<int> chemin;
    int distance();
    int destination();
};



#endif
