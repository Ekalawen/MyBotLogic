#ifndef BT_NOEUD_H
#define BT_NOEUD_H
class BT_Noeud
{
public:
   enum ETAT_ELEMENT{ECHEC,EN_COURS,REUSSI};
   virtual ETAT_ELEMENT execute() = 0;
};

#endif

