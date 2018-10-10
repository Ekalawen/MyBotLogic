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
      while (elements[ind].execute() != Reussi) {

      }
   }
};