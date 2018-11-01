#include "MapPosition.h"

#include <algorithm>

const std::vector<MapPosition> MapPosition::mouvementDeBase =
{
    { 1,-1 }, //NE
    { 1, 0 }, //E
    { 0, 1 }, //SE
    { -1, 1 },//SW
    { -1, 0 },//W
    { 0, -1 } //NW
};

Tile::ETilePosition MapPosition::getDirectionInverse(const Tile::ETilePosition _direction) const noexcept {
    //direction + 3 % 6, on peut ignorer CENTRE étant donné qu'il n'y a pas d'inverse
    return static_cast<Tile::ETilePosition>(
        (_direction + mouvementDeBase.size() / 2) % mouvementDeBase.size());
}

MapPosition::MapPosition(const int _x, const int _z)
    : x{ _x }, z{ _z }
{}

inline MapPosition MapPosition::CalculerPositionDeplacement(const Tile::ETilePosition _direction) const noexcept {
    return mouvementDeBase[_direction];
}

MapPosition MapPosition::CalculatePositionResultante(const Tile::ETilePosition _direction) const noexcept {
    return { *this + CalculerPositionDeplacement(_direction) };
}

MapPosition MapPosition::CalculerPositionOffset(const Tile::ETilePosition _direction) const noexcept {
    return CalculatePositionResultante(_direction).AxialToOffset();
}

MapPosition MapPosition::AxialToOffset() const noexcept {
    return { x + (z - (z & 1)) / 2, z };
}

float MapPosition::DistancreReelEntre(const MapPosition& _autre) const noexcept {
    auto resultatY = -x - z - -_autre.x - _autre.z;
    return static_cast<float>(
        sqrt((x - _autre.x) * (x - _autre.x) + resultatY * resultatY + (z - _autre.z) * (z - _autre.z)));
}

unsigned int MapPosition::NbTuilesEntre(const MapPosition& _autre) const noexcept {
    return static_cast<unsigned int>(
        std::max(
            std::max(abs(x - _autre.x), abs(z - _autre.z)),
            abs((-x - z) - (-_autre.x - _autre.z))));
}