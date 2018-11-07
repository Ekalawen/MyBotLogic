#ifndef PORTE_H
#define PORTE_H

#include "ObjectInfo.h"
#include "Map.h"
#include <vector>
using namespace std;

class not_a_door {};

class Map;
class Porte {
public:
    enum Type { A_SWITCH, A_POIGNEE };
private:
    int id;
    Type type;
    Object::EObjectState etat;
    vector<int> tilesVoisines; // Vector de taille maximum 2 !
    int switchId; // L'identifiant de l'activateur si il s'agit d'une porte à switch !
public:
    Porte() = default;
    Porte(const ObjectInfo& porte, const Map& m);

    int getId() const noexcept;
    Type getType() const noexcept;
    Object::EObjectState getEtat() const noexcept;
    vector<int> getTilesVoisines() const noexcept;
    bool isVoisine(const int tileId) const noexcept;
    int getSwitchId() const noexcept;

    void ouvrirPorte(); // Permet d'ouvrir une porte :D
    void fermerPorte(); // Permet de ... fermer une porte ! :D
};


#endif



