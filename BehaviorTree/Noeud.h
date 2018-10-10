#ifndef NOEUD_H
#define NOEUD_H
#include "Element.h"
#include <vector>
using namespace std;
class Noeud : Element
{
public:
   vector<Element> elements;
};
#endif

class Selectionneur : Noeud {
   Element::ETAT_ELEMENT execute() {
      int ind = 0;
      Element::ETAT_ELEMENT res = elements[ind].execute();
      while (res == Element::ETAT_ELEMENT::ECHEC) {
         res = elements[++ind].execute();
      }
      return res;
   }
};

class Sequenceur : Noeud {
   Element::ETAT_ELEMENT execute() {
      int ind = 0;
      Element::ETAT_ELEMENT res = elements[ind].execute();
      while (res == Element::ETAT_ELEMENT::REUSSI) {
         res = elements[++ind].execute();
      }
      return res;
   }
};