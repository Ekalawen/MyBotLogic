#ifndef NOEUD_H
#define NOEUD_H
class Noeud
{
public:
   enum ETAT_ELEMENT{ECHEC,EN_COURS,REUSSI};
   virtual ETAT_ELEMENT execute() = 0;
};

#endif

