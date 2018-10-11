#ifndef COMPOSITE_H
#define COMPOSITE_H
#include "Noeud.h"
#include <vector>
using namespace std;
class Composite : public Noeud
{
public:
   vector<Noeud> elements;
};
#endif

class Selectionneur : public Composite {
   Noeud::ETAT_ELEMENT execute() {
      int ind = 0;
      Noeud::ETAT_ELEMENT res = elements[ind].execute();
      while (ind < elements.size() && res == Noeud::ETAT_ELEMENT::ECHEC) {
         res = elements[++ind].execute();
      }
      return res;
   }
};

class Sequenceur : public Composite {
   Noeud::ETAT_ELEMENT execute() {
      int ind = 0;
      Noeud::ETAT_ELEMENT res = elements[ind].execute();
      while (ind < elements.size() &&  res == Noeud::ETAT_ELEMENT::REUSSI) {
         res = elements[++ind].execute();
      }
      return res;
   }
};