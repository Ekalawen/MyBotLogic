#include "Mouvement.h"
#include "GameManager.h"
#include "Porte.h"
#include <sstream>
using std::stringstream;

Mouvement::Mouvement(const int npcID, const int tileSource, const int tileDestination, const Tile::ETilePosition direction)
: npcID(npcID), tileSource(tileSource), tileDestination(tileDestination), direction(direction)
{
}

void Mouvement::stop() noexcept {
    direction = Tile::CENTER;
    tileDestination = tileSource;
}

bool Mouvement::isNotStopped() const noexcept {
    return direction != Tile::CENTER;
}

int Mouvement::getNpcId() const noexcept {
    return npcID;
}

int Mouvement::getTileSource() const noexcept {
    return tileSource;
}

int Mouvement::getTileDestination() const noexcept {
    return tileDestination;
}

Tile::ETilePosition Mouvement::getDirection() const noexcept {
    return direction;
}

void Mouvement::setActivateDoor() noexcept {
    isActivateDoorMouvement = true;
}

void Mouvement::apply(vector<Action*>& actionList, map<int, Npc>& npcs, Carte& c) const noexcept {
    // On ne prend en compte notre mouvement que s'il compte
    if (!isActivateDoorMouvement) {
        if (isNotStopped()) {
            // ET ENFIN ON FAIT BOUGER NOTRE NPC !!!!! <3
            actionList.push_back(new Move(getNpcId(), getDirection()));
            // ET ON LE FAIT AUSSI BOUGER DANS NOTRE MODELE !!!
            npcs[getNpcId()].move(getDirection(), c);
            // TEST : pour chaque npc qui se déplace sur son objectif à ce tour, alors mettre estArrive à vrai
            if (getDirection() != Tile::ETilePosition::CENTER && npcs[getNpcId()].getTileObjectif() == getTileDestination())
                // il faut aussi vérifier si tous les NPC ont un objectif atteignable, donc si on est en mode Exploitation
            {
                npcs[getNpcId()].setArrived(true);
            }
            else {
                npcs[getNpcId()].setArrived(false);
            }
        }
    } else {
        // Si notre mouvement est en fait un mouvement d'activation de porte ...
        // On récupère l'id de la porte
        Porte& porte = c.getPorte(getTileSource(), getTileDestination());
        // On ouvre la porte dans notre modèle
        porte.ouvrirPorte();
        // On envoie l'évènement d'ouverture de porte !
        actionList.push_back(new Interact(getNpcId(), porte.getId(), Interact::EInteraction::Interaction_OpenDoor));
        stringstream ss; ss << "Le Npc " << getNpcId() << " a ouvert la porte " << porte.getId() << " !";
        GAME_MANAGER_LOG_DEBUG(ss.str());
    }
}