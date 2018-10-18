#ifndef BT_NOEUD_H
#define BT_NOEUD_H

class BT_Noeud {
public:
   enum ETAT_ELEMENT{ECHEC, REUSSI, EN_COURS,};
   BT_Noeud() = default;
   virtual ETAT_ELEMENT execute() noexcept = 0;
};

#endif

