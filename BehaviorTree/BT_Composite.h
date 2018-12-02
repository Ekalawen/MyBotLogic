#ifndef BT_COMPOSITE_H
#define BT_COMPOSITE_H

#include "BT_Noeud.h"

#include <vector>
#include <memory>

using std::vector;
using std::unique_ptr;
using std::move;

class BT_Composite : public BT_Noeud {
public:
   vector<unique_ptr<BT_Noeud>> noeuds;

   BT_Composite() = default;
   BT_Composite(vector<unique_ptr<BT_Noeud>>&& _noeuds) : noeuds{ move(_noeuds) } {
   }

   virtual ETAT_ELEMENT execute() noexcept override = 0;
};

#endif