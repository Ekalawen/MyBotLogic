#ifndef SEQUENCEUR_H
#define SEQUENCEUR_H

#include "../BT_Composite.h"

#include <memory>

using std::vector;
using std::unique_ptr;
using std::move;

class Sequenceur : public BT_Composite {

public:

   Sequenceur() = default;
   Sequenceur(vector<unique_ptr<BT_Noeud>>&& _noeuds) : BT_Composite(move(_noeuds)) {
   }
  
   BT_Noeud::ETAT_ELEMENT execute() noexcept override {
      int ind = 0;
      BT_Noeud::ETAT_ELEMENT res;
	  do {
		  res = noeuds[ind]->execute();
	  } while (++ind < noeuds.size() && res == BT_Noeud::ETAT_ELEMENT::REUSSI);
      
      return res;
   }
};
#endif