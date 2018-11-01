#ifndef MAP_POSITION_H
#define MAP_POSITION_H

#include "Globals.h"

#include <vector>

class MapPosition {
public:
    static const std::vector<MapPosition> mouvementDeBase;

    int x, z;

    MapPosition(const int x, const int z);

    Tile::ETilePosition getDirectionInverse(const Tile::ETilePosition _direction) const noexcept;

    MapPosition CalculerPositionDeplacement(const Tile::ETilePosition _direction) const noexcept;
    MapPosition CalculatePositionResultante(const Tile::ETilePosition _direction) const noexcept;
    MapPosition CalculerPositionOffset(const Tile::ETilePosition _direction) const noexcept;
    MapPosition AxialToOffset() const noexcept;

    float DistancreReelEntre(const MapPosition& _autre) const noexcept;
    unsigned int NbTuilesEntre(const MapPosition& _autre) const noexcept;

    const bool operator==(const MapPosition& other) const noexcept {
        return x == other.x && z == other.z;
    }
    const MapPosition operator+(const MapPosition& other) const noexcept {
        return { x + other.x, z + other.z };
    }
    const MapPosition operator*(const int& scale) const noexcept {
        return { x * scale, z * scale };
    }
};

#endif