#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H
#include "BT_Noeud.h"
#include <vector>
using namespace std;
class BT_Composite : public BT_Noeud
{
public:
   vector<BT_Noeud> elements;
};
#endif

//class Selectionneur : public BT_Composite {
//   BT_Noeud::ETAT_ELEMENT execute() {
//      int ind = 0;
//      BT_Noeud::ETAT_ELEMENT res = elements[ind].execute();
//      while (ind < elements.size() && res == BT_Noeud::ETAT_ELEMENT::ECHEC) {
//         res = elements[++ind].execute();
//      }
//      return res;
//   }
//};
//
//class Sequenceur : public BT_Composite {
//   BT_Noeud::ETAT_ELEMENT execute() {
//      int ind = 0;
//      BT_Noeud::ETAT_ELEMENT res = elements[ind].execute();
//      while (ind < elements.size() &&  res == BT_Noeud::ETAT_ELEMENT::REUSSI) {
//         res = elements[++ind].execute();
//      }
//      return res;
//   }
//};