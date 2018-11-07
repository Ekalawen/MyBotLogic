#ifndef SELECTEUR_H
#define SELECTEUR_H

#include "../BT_Composite.h"

#include <memory>

using std::vector;
using std::unique_ptr;
using std::move;

class Selecteur : public BT_Composite {

public:
   Selecteur() = default;
   Selecteur(vector<unique_ptr<BT_Noeud>>&& _noeuds) : BT_Composite(move(_noeuds)) {
   }

   BT_Noeud::ETAT_ELEMENT execute() noexcept override {
	   int ind = 0;
	   BT_Noeud::ETAT_ELEMENT res;
	   do {
		   res = noeuds[ind]->execute();
	   } while (++ind < noeuds.size() && res == BT_Noeud::ETAT_ELEMENT::ECHEC);

      return res;
   }
};

#endif