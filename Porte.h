#ifndef PORTE_H
#define PORTE_H

#include "Globals.h"
#include "Contrainte.h"
#include "Npc.h"

#include <vector>
#include <set>

class not_a_door {};

class Carte;
class GameManager;
struct ObjectInfo;
class Npc;
class Contrainte;

class Porte {
public:
    enum Type { A_SWITCH, A_POIGNEE };
private:
    int id;
    Type type;
    Object::EObjectState etat;
    std::vector<int> tilesVoisines; // Vector de taille maximum 2 !
    std::set<int> switchsIds; // L'identifiant de l'activateur si il s'agit d'une porte � switch !

public:
    Porte() = default;
    Porte(const ObjectInfo& porte, const Carte& c);

    int getId() const noexcept;
    Type getType() const noexcept;
    Object::EObjectState getEtat() const noexcept;
    std::vector<int> getTilesVoisines() const noexcept;
    bool isVoisine(const int tileId) const noexcept;
    std::set<int> getSwitchsIds() const noexcept;
    std::set<int> getSwitchsTilesIds(const Carte& c) const noexcept;

    void ouvrirPorte(); // Permet d'ouvrir une porte :D
    void fermerPorte(); // Permet de ... fermer une porte ! :D

    // Cette fonction renvoie vrai si un npc pourrait traverser cette porte depuis la caseAvantPorte si d'autre npcs qui ne sont pas occup�es l'aidaient.
    // Elle stocke aussi le temps n�cessaire d'attente avant l'ouverture de la porte dans tempsAvantOuverture. C'est un param�tre de retour.
    // Et les contraintes n�cessaires, qui sont faisables et r�solues, pour passer cette porte. C'est un param�tre de retour.
    bool canPassDoor(const int npcActif, const int caseAvantPorte, GameManager& gm, int& tempsAvantOuverture, std::vector<Contrainte>& contraintesNecessaires) const noexcept;

    //            if (!gm.c.getTile(courant.tile.getId()).hasDoorSwitch(voisin, gm.c) // On v�rifie qu'il n'y a pas de porte � switch devant
    //                || gm.c.getTile(courant.tile.getId()).getContrainte(voisin, gm.c).isSolvableWithout(vector<int>{getId()}, gm, cout)) { // O� qu'elle n'est pas g�nante !

private:
   bool switchAccessible(const int caseAvantPorte, GameManager& gm, std::vector<Contrainte>& contraintesDejaNecessaires, const int switchTileId, const std::set<int>& switchsTilesIds, const Npc& npcSuppleant, std::vector<Contrainte>& contraintesFinales) const noexcept;
};


#endif



