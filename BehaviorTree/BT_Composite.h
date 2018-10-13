#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H
#include "BT_Noeud.h"
#include <vector>
using namespace std;
class BT_Composite : public BT_Noeud
{
public:
   BT_Composite(vector<BT_Noeud*> noeuds) : noeuds{ noeuds } {
   }
   virtual ETAT_ELEMENT execute() override = 0;
public:
   vector<BT_Noeud*> noeuds;
};
#endif