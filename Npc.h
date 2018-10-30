#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Chemin.h"
#include "Carte.h"
#include <vector>

class tile_inaccessible {};

class Carte;
class Npc {
private:
   std::vector<Chemin> cheminsPossibles; // Ceci est une variable temporaire permettant de stocker les chemins parmis lesquelles choisirs un objectif
   std::map<int, float> scoresAssocies; // Les scores associés aux tiles !

   int id;
   int tileId; // Sa position sur la carte
   int tileObjectif; // Là où il doit aller !
   Chemin chemin; // Utilisé pour savoir quel chemin suivre pour se rendre à l'objectif
   std::vector<int> ensembleAccessible; // ensemble des tuiles auquel un npc à accès
   std::map<int, int> distancesEnsembleAccessible;
   bool estArrive; // indique si le npc a atteind son objectif

public:

   Npc() = default;
   Npc(const NPCInfo);

   void move(Tile::ETilePosition, Carte&) noexcept; // Permet de faire bouger notre npc dans notre modèle =)

   void resetChemins() noexcept;
   void addChemin(Chemin& _chemin) noexcept;
   void addScore(int _tileIndice, float _score) noexcept;
   Chemin getCheminMinNonPris(std::vector<int> _objectifsPris, int _tailleCheminMax) const noexcept; // Permet de trouver le chemin le plus court qui ne soit pas déjà pris
   int affecterMeilleurChemin(Carte &_map) noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
   void floodfill(Carte &_map); // Calcule le coût et l'ensemble des tiles accessibles pour un npcs, et MAJ ses attributs.

   int getId();
   int getTileId();
   int getTileObjectif();
   void setTileObjectif(int _idTile);
   Chemin& getChemin();
   std::vector<int> getEnsembleAccessible();
   bool isAccessibleTile(int _tileId);
   int distanceToTile(int _tileId);
   std::map<int, int> getDistancesEnsembleAccessible();
   bool isArrived();
   void setArrived(bool _etat);
};

#endif