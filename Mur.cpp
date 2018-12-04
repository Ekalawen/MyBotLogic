
#include "Mur.h"
#include "Carte.h"
#include <algorithm>
using namespace std;

Mur::Mur(const int id, const int tileID, Tile::ETilePosition position, set<Object::EObjectType> objectTypes, const Carte& carte) :
    id{ id }, tileID{ tileID }, position{ position }, objectTypes{ objectTypes }
{
    // On précalcule les tiles voisines !
    tilesVoisinesId.push_back(tileID);
    int deuxiemeVoisine = carte.getAdjacentTileAt(tileID, position);
    if (deuxiemeVoisine != -1)
        tilesVoisinesId.push_back(deuxiemeVoisine);
}

int Mur::getId() const noexcept {
    return id;
}

vector<int> Mur::getTilesVoisines() const noexcept {
    return tilesVoisinesId;
}

bool Mur::isBetween(const int idVoisine1, const int idVoisine2) {
    return (find(tilesVoisinesId.begin(), tilesVoisinesId.end(), idVoisine1) != tilesVoisinesId.end()
        && find(tilesVoisinesId.begin(), tilesVoisinesId.end(), idVoisine2) != tilesVoisinesId.end());
}

set<Object::EObjectType> Mur::getTypes() const noexcept {
    return objectTypes;
}

bool Mur::hasBeenVerified() {
    return aEteVerifie;
}
void Mur::verify(Carte& c) {
    setHasBeenVerified();
    c.addMurCheckedId(getId());
    c.removeWall(getId());
}

void Mur::setHasBeenVerified() noexcept {
    aEteVerifie = true;
}
