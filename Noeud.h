#ifndef NOEUD_H
#define NOEUD_H

#include "MapTile.h"
#include <map>
#include <vector>

struct Noeud {
    static float coefEvaluation;
    MapTile tile;
    float cout; // La distance calculé depuis le départ
    float evaluation; // La distance estimée à l'arrivée
    float heuristique; // La somme du cout et de l'evaluation
    int idPrecedant;

    Noeud() = default;
    Noeud(const MapTile& tile, const float cout, const float evaluation, const int idPrecedant) // Constructeur complet
        : tile{ tile }, cout{ cout }, evaluation{ evaluation }, idPrecedant{ idPrecedant } {
        heuristique = cout + evaluation * coefEvaluation;
    }
    Noeud(const MapTile& tile, const float cout) : // Constructeur minimaliste
        tile{ tile }, cout{ cout }, evaluation{ 0 }, idPrecedant{ -1 } {
        heuristique = cout + evaluation * coefEvaluation;
    }

    friend bool operator==(const Noeud& g, const Noeud& d) {
        return g.tile.getId() == d.tile.getId();
    }
};



#endif
