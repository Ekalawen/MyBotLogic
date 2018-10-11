#ifndef BT_FEUILLE_H
#define BT_FEUILLE_H

#include "BT_Noeud.h"

class BT_Feuille : public BT_Noeud
{
public:
   BT_Feuille() = default;
   virtual ETAT_ELEMENT execute() = 0;
};

#endif

