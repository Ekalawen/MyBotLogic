#ifndef NPC_H
#define NPC_H

#include "Globals.h"
#include "NPCInfo.h"
#include "Map.h"
#include "Chemin.h"
#include <vector>
using namespace std;

class Npc {
    vector<Chemin> cheminsPossibles; // Ceci est une variable temporaire permettant de stocker les chemins parmis lesquelles choisirs un objectif
    vector<float> scoresAssocies; // Les scores associes aux chemins !
public:
    int id;
    int tileId; // Sa position sur la carte
    int tileObjectif; // Là où il doit aller !
    Chemin chemin; // Utilisé pour savoir quel chemin suivre pour se rendre à l'objectif
    Npc() = default;
    Npc(const NPCInfo);

    void move(Tile::ETilePosition, Map) noexcept; // Permet de faire bouger notre npc dans notre modèle =)

    void resetChemins() noexcept;
    void addChemin(Chemin& chemin) noexcept;
    void addCheminWithScore(Chemin& chemin, float score) noexcept;
    Chemin getCheminMinNonPris(vector<int> objectifsPris, int tailleCheminMax) const noexcept; // Permet de trouver le chemin le plus court qui ne soit pas déjà pris
    int affecterMeilleurChemin() noexcept; // Affecte au npc le chemin avec le meilleur score et renvoie la destination de ce chemin !
};



#endif
