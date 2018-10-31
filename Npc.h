#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Chemin.h"
#include "Map.h"
#include <vector>
using namespace std;

class tile_inaccessible {};

struct Score {
    int tuileID;
    float score;

    Score(const int _tuileID, const float _score)
        :tuileID{ _tuileID }, score{ _score }
    {}
    ~Score() = default;
};

class Map;
class Npc {
private:
    vector<Chemin> cheminsPossibles; // Ceci est une variable temporaire permettant de stocker les chemins parmis lesquelles choisirs un objectif
    vector<Score> scoresAssocies; // Les scores associ�s aux tiles !

    int id;
    int tileId; // Sa position sur la carte
    int tileObjectif; // L� o� il doit aller !
    Chemin chemin; // Utilis� pour savoir quel chemin suivre pour se rendre � l'objectif
    vector<int> ensembleAccessible; // ensemble des tuiles auquel un npc � acc�s
    map<int, int> distancesEnsembleAccessible;
	bool estArrive; // indique si le npc a atteind son objectif

public:

    Npc() = default;
    Npc(const NPCInfo);

    void move(Tile::ETilePosition, Map&) noexcept; // Permet de faire bouger notre npc dans notre mod�le =)

    void resetChemins() noexcept;
    void addChemin(Chemin& chemin) noexcept;
    void addScore(Score score) noexcept;
    Chemin getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept; // Permet de trouver le chemin le plus court qui ne soit pas d�j� pris
    int affecterMeilleurChemin(Map &m) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
    void floodfill(Map &m); // Calcule le co�t et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.

    int getId();
    int getTileId();
    int getTileObjectif();
    void setTileObjectif(int idTile);
    Chemin& getChemin();
    vector<int> getEnsembleAccessible();
    bool isAccessibleTile(int tileId);
    int distanceToTile(int tileId);
    bool isArrived();
    void setArrived(bool etat);
};

#endif