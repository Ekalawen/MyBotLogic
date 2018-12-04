#ifndef MUR_H
#define MUR_H

#include "Globals.h"
#include <vector>
#include <set>
using std::vector;
using std::set;

class Carte;
class Mur {
private:
    int id;
    int tileID;
	Tile::ETilePosition position;
	set<Object::EObjectType> objectTypes; // On peut avoir des murs vitrés et des portes vitrés
    vector<int> tilesVoisinesId{};
    bool aEteVerifie = false; // Pour savoir si il y a une porte cachée à l'intérieur

public:
    Mur() = default;
    Mur(const int id, const int tileID, Tile::ETilePosition position, set<Object::EObjectType> objectTypes, const Carte& carte);

    int getId() const noexcept;
    vector<int> getTilesVoisines() const noexcept;
    bool isBetween(const int idVoisine1, const int idVoisine2); // Permet de savoir si le mur est entre ces deux tuiles adjacentes
    set<Object::EObjectType> getTypes() const noexcept;
    bool hasBeenVerified(); // Permet de savoir si il était en fait une porte cachée ou non
    void verify(Carte& c); // Met à jour le modèle pour savoir que l'on a vérifiée ce mur !
    void setHasBeenVerified() noexcept;
};

#endif



