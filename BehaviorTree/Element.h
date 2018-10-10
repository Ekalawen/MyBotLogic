#ifndef ELEMENT_H
#define ELEMENT_H
class Element
{
public:
enum ETAT_ELEMENT{ECHEC,EN_COURS,REUSSI};
virtual ETAT_ELEMENT execute() = 0;
};

#endif

