#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H
#include "BT_Noeud.h"
#include <vector>

class BT_Composite : public BT_Noeud {
public:
   std::vector<BT_Noeud*> noeuds;

   BT_Composite() = default;
   BT_Composite(std::vector<BT_Noeud*> _noeuds) : noeuds{ _noeuds } {
   }

   virtual ETAT_ELEMENT execute() noexcept override = 0;
};

#endif