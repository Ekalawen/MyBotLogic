#ifndef CHEMIN_H
#define CHEMIN_H

#include <vector>
using namespace std;

// Un chemin est constitu� d'un vecteur contenant les indices des cases adjacentes sur lesquelles se d�placer !
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
