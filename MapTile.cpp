
#include "MapTile.h"
#include "GameManager.h"

#include <algorithm>

MapTile::MapTile(unsigned int _id, MapPosition& _position) :
    id{ static_cast<int>(_id) },
    position{ _position },
    type{ Tile::ETileType::TileAttribute_Default },
    statut{ INCONNU }
{
    voisins.reserve(6);
}

void MapTile::setTileDecouverte(const TileInfo& tile) noexcept {
   type = tile.tileType;
   statut = CONNU;
}

bool MapTile::isVoisinAvecEtat(const Etats etat, const int id) const noexcept {
    return std::find_if(voisins.begin(), voisins.end(), [&](const Voisin& v) {
        return v.getTuileIndex() == id && v.estEtat(etat);
    }) != voisins.end();
}

void MapTile::removeEtat(const Etats etat, const int id) {
    auto it = std::find_if(voisins.begin(), voisins.end(), [&id](const Voisin& v) {
        return v.getTuileIndex() == id;
    });

    if (it != voisins.end()) {
        it->setEtat(etat, false);
    }
}

bool MapTile::existe() const noexcept {
   return statut != MapTile::Statut::INCONNU;
}

int MapTile::getId() const noexcept {
    return id;
}

MapPosition MapTile::getPosition() const noexcept {
    return position;
}

Tile::ETileType MapTile::getType() const noexcept {
    return type;
}

vector<Voisin> MapTile::getVoisins() const noexcept {
    return voisins;
}

vector<Voisin>& MapTile::getVoisins() noexcept {
    return voisins;
}

vector<int> MapTile::getVoisinsIDParEtat(const Etats etat) const noexcept {

    vector<int> resultat;

    std::for_each(begin(voisins), end(voisins), [&](const Voisin& v) {
        if (v.estEtat(etat))
            resultat.emplace_back(v.getTuileIndex());
    });

    return resultat;
}

MapTile::Statut MapTile::getStatut() const noexcept {
    return statut;
}

void MapTile::setStatut(MapTile::Statut new_statut) {
    statut = new_statut;
}
