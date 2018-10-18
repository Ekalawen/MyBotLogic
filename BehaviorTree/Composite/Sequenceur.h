#ifndef SEQUENCEUR_H
#define SEQUENCEUR_H

#include "../BT_Composite.h"

class Sequenceur : public BT_Composite {

public:

   Sequenceur() = default;
   Sequenceur(vector<BT_Noeud*> noeuds) : BT_Composite(noeuds) {
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