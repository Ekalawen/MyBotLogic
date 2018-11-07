#include "MyBotLogic/Porte.h"
#include "ObjectInfo.h"
#include "Carte.h"

Porte::Porte(const ObjectInfo& porte, const Carte& c) {
    // On vérifie que notre objet est bien une porte
    if (find(porte.objectTypes.begin(), porte.objectTypes.end(), Object::ObjectType_Door) == porte.objectTypes.end())
        throw not_a_door{};

    // On set les attributs de base
    id = porte.objectID;
    int voisin1 = porte.tileID;
	int voisin2 = c.getAdjacentTileAt(porte.tileID, porte.position);
    tilesVoisines.push_back(voisin1);
    if(voisin2 != -1)
        tilesVoisines.push_back(voisin2);
    if (find(porte.objectStates.begin(), porte.objectStates.end(), Object::ObjectState_Opened) != porte.objectStates.end())
        etat = Object::ObjectState_Opened;
    else
        etat = Object::ObjectState_Closed;
    switchId = -1;
    if (porte.connectedTo.size() == 0)
        type = A_POIGNEE;
    else
        type = A_SWITCH;
}

int Porte::getId() const noexcept {
    return  id;
}
Porte::Type Porte::getType() const noexcept {
    return type;
}
Object::EObjectState Porte::getEtat() const noexcept {
    return etat;
}
vector<int> Porte::getTilesVoisines() const noexcept {
    return tilesVoisines;
}
bool Porte::isVoisine(const int tileId) const noexcept {
    return find(tilesVoisines.begin(), tilesVoisines.end(), tileId) != tilesVoisines.end();
}
int Porte::getSwitchId() const noexcept {
    return switchId;
}

void Porte::ouvrirPorte() {
    etat = Object::ObjectState_Opened;
}

void Porte::fermerPorte() {
    etat = Object::ObjectState_Closed;
}
