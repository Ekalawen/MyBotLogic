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
    int distance() noexcept;
    int destination() noexcept;
    void setInaccessible() noexcept;
    bool isAccessible() noexcept;
    bool empty() noexcept;
    string toString() noexcept;
};



#endif
