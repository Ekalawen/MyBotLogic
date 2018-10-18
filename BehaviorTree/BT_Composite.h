#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H
#include "BT_Noeud.h"
#include <vector>
using namespace std;

class BT_Composite : public BT_Noeud {
public:
   vector<BT_Noeud*> noeuds;

   BT_Composite() = default;
   BT_Composite(vector<BT_Noeud*> noeuds) : noeuds{ noeuds } {
   }

   virtual ETAT_ELEMENT execute() noexcept override = 0;
};

#endif