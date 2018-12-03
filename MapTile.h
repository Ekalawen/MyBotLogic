#ifndef MAP_TILE_H
#define MAP_TILE_H

#include "TileInfo.h"
#include "Voisin.h"
#include "Contrainte.h"
#include <vector>

using std::vector;

class contrainte_inexistante {};

class Carte;
class MapTile {
public:
    enum Statut { INCONNU, CONNU, VISITE, PRESUME_CONNU }; // Présumé_connu est utilisé pour les portes. On sait qu'il y a quelque chose derrière, mais on sait pas ce que c'est !

private:
    int id;
    int x, y; // La position de la tile. x est l'indice de colonne, y est l'indice de ligne.
    Tile::ETileType type;
    vector<Voisin> voisins;
    Statut statut;
    vector<int> portesAdjacentes;

public:
    MapTile() = default; // Constructeur par défaut obligatoire pour pouvoir utiliser tuple ...
    MapTile(unsigned int id, Carte &m); // Appelé dés le début et uniquement là !

    void setTileDecouverte(const TileInfo& _tile) noexcept;
    void presumerConnu() noexcept;

    void removeEtat(const Etats etat, const int id);
	bool isVoisinAvecEtat(const Etats etat, const int id) const noexcept;

    bool existe() const noexcept;

    int getId() const noexcept;
    int getX() const noexcept;
    int getY() const noexcept;
    Tile::ETileType getType() const noexcept;
    vector<Voisin> getVoisins() const noexcept;
    vector<int> getVoisinsId() const noexcept;
    vector<int> getVoisinsIDParEtat(const Etats etat) const noexcept;
    bool hasDoorPoigneeVoisin(const int voisinId, const Carte& c) const noexcept; // Permet de savoir s'il y a une porte à poignée fermée entre cette tile et sa voisine.
    bool hasDoor(const int voisinId, const Carte& c) const noexcept; // Permet de savoir s'il y a une porte
    bool hasClosedDoor(const int voisinId, const Carte& c) const noexcept; // Permet de savoir s'il y a une porte fermée
    bool hasClosedDoorSwitch(const int voisinId, const Carte& c) const noexcept; // Permet de savoir s'il y a une porte à switch fermée entre cette tile et sa voisine.
    bool canPassDoor(const int tileVoisineId, const int npcActif, const int caseAvantPorte, GameManager& gm, int& tempsAvantOuverture, vector<Contrainte>& contraintesNecessaires) const noexcept;
    Contrainte getContrainte(const int voisinId, const Carte& c) const; // Retourne la contrainte associée à ce voisin si elle existe !
    Statut getStatut() const noexcept;
    void setStatut(Statut new_statut);
    void addPorte(int porteId);
};

#endif
