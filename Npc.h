#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Chemin.h"
#include "Carte.h"
#include <vector>

using std::vector;

class tile_inaccessible {};

template <class T>
class Score {
public:
    int tuileID;
    T score;

    Score(const int _tuileID, const T _score)
        :tuileID{ _tuileID }, score{ _score }
    {}
    ~Score() = default;
};

using ScoreType = Score<float>;
using Scores = vector<ScoreType>;

using DistanceType = Score<int>;
using Distances = vector<DistanceType>;

class Carte;
class GameManager;
class Npc {
private:
    vector<int> objectifsPossibles; // Ceci est une variable temporaire permettant de stocker les objectifs parmis lesquels choisir
    //vector<Chemin> cheminsPossibles; // Ceci est une variable temporaire permettant de stocker les chemins parmis lesquelles choisirs un objectif
    Scores scoresAssocies; // Les scores associés aux tiles !
    Distances ensembleAccessible; // ensemble des tuiles auquel un npc à accés avec la distance

    int id;
    int tileId; // Sa position sur la carte
    int tileObjectif; // Là oû il doit aller !
    Chemin chemin; // Utilisé pour savoir quel chemin suivre pour se rendre à l'objectif

    //Distances distancesEnsembleAccessible;
	bool estArrive; // indique si le npc a atteind son objectif
    bool isCheckingDoor = false; // Permet de savoir si le npc doit checker une porte ou non !
    Tile::ETilePosition doorCheckingDirection = Tile::CENTER; // La direction dans laquelle il doit checker !

public:

    Npc() = default;
    Npc(const NPCInfo);

    void move(const Tile::ETilePosition, Carte&) noexcept; // Permet de faire bouger notre npc dans notre modèle =)

    void resetObjectifs() noexcept;
    void addObjectif(const int tileIDObjectif) noexcept;
    //void resetChemins() noexcept;
    //void addChemin(Chemin& chemin) noexcept;
    void addScore(ScoreType score) noexcept;
    int getObjectifMinNonPris(const vector<int>& objectifsPris, const int tailleCheminMax) const noexcept; // Permet de trouver l'objectif le plus court qui ne soit pas déjà pris
    int affecterMeilleurObjectif(GameManager& gm) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
    void floodfill(GameManager& gm); // Calcule le coût et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.
    Scores::iterator chercherMeilleurScore(Scores& _scores);

    int getId() const noexcept;
    int getTileId() const noexcept;
    int getTileObjectif() const noexcept; // Permet de savoir où veut aller notre npc ! :)
    void setTileObjectif(const int idTile) noexcept;
    Chemin& getChemin() noexcept;
    void setChemin(Chemin& _chemin) noexcept;
    Distances& getEnsembleAccessible() noexcept;
    bool isAccessibleTile(const int tileId) const noexcept;
    int distanceToTile(const int tileId) const ;
    bool isArrived() const noexcept;
    void setArrived(const bool etat) noexcept;
    void setIsCheckingDoor(bool etat, Tile::ETilePosition direction = Tile::CENTER);
    bool getIsCheckingDoor() const noexcept;
    Tile::ETilePosition getDirectionCheckingDoor() const noexcept;
};

#endif