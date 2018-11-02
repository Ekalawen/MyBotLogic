#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Chemin.h"
#include "Carte.h"

#include <vector>

class tile_inaccessible {};

template <class T> class Score
{
public:
    int tuileID;
    T score;

    Score(const int _tuileID, const T _score)
        :tuileID{ _tuileID }, score{ _score }
    {}
    ~Score() = default;
};

using ScoreType = Score<float>;
using Scores = std::vector<ScoreType>;

using DistanceType = Score<int>;
using Distances = std::vector<DistanceType>;

class Carte;

class Npc {
private:
    std::vector<Chemin> cheminsPossibles; // Ceci est une variable temporaire permettant de stocker les chemins parmis lesquelles choisirs un objectif
    Scores scoresAssocies; // Les scores associ�s aux tiles !
    Distances ensembleAccessible; // ensemble des tuiles auquel un npc � acc�s avec la distance

    int id;
    int tileId; // Sa position sur la carte
    int tileObjectif; // L� o� il doit aller !
    Chemin chemin; // Utilis� pour savoir quel chemin suivre pour se rendre � l'objectif

    //Distances distancesEnsembleAccessible;
	bool estArrive; // indique si le npc a atteind son objectif

public:

    Npc() = default;
    Npc(const NPCInfo);

    void move(const Tile::ETilePosition, Carte&) noexcept; // Permet de faire bouger notre npc dans notre mod�le =)

    void resetChemins() noexcept;
    void addChemin(Chemin& chemin) noexcept;
    void addScore(ScoreType score) noexcept;
    Chemin getCheminMinNonPris(const std::vector<int>& _objectifsPris, const int _tailleCheminMax) const noexcept; // Permet de trouver le chemin le plus court qui ne soit pas d�j� pris
    int affecterMeilleurChemin(const Carte& _carte) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
    void floodfill(const Carte& _carte); // Calcule le co�t et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.

    int getId() const noexcept;
    int getTileId() const noexcept;
    int getTileObjectif() const noexcept;
    void setTileObjectif(const int idTile) noexcept;
    Chemin& getChemin() noexcept;
    Distances& getEnsembleAccessible() noexcept;
    bool isAccessibleTile(const int tileId) const noexcept;
    int distanceToTile(const int tileId);
    bool isArrived() const noexcept;
    void setArrived(const bool etat) noexcept;
};

#endif