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

void Mouvement::setActivateDoor(Tile::ETilePosition direction) noexcept {
    isActivateDoorMouvement = true;
    directionActivateDoorMouvement = direction;
}

void Mouvement::setCheckingDoor(Tile::ETilePosition direction) noexcept {
    isCheckingDoor = true;
    directionCheckingDoor = direction;
}

void Mouvement::apply(vector<Action*>& actionList, map<int, Npc>& npcs, Carte& c) const noexcept {
    // On vérifie si c'est un mouvement de checking de door
    if (isCheckingDoor) {
        // On envoie l'action de checker un mur
        Mur& mur = c.getMurInDirection(getTileSource(), directionCheckingDoor);
        actionList.push_back(new Interact(getNpcId(), mur.getId(), Interact::EInteraction::Interaction_SearchHiddenDoor));

        stringstream ss; ss << "Le Npc " << getNpcId() << " a checker le mur " << mur.getId() << " !";
        LOG(ss.str());

        // On précise que l'on a ouvert ce mur dans notre modèle
        mur.verify(c);
        return;
    }

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
        int tileDestination = c.getAdjacentTileAt(getTileSource(), directionActivateDoorMouvement);
        Porte& porte = c.getPorte(getTileSource(), tileDestination);
        // On ouvre la porte dans notre modèle
        porte.ouvrirPorte();
        // On envoie l'évènement d'ouverture de porte !
        actionList.push_back(new Interact(getNpcId(), porte.getId(), Interact::EInteraction::Interaction_OpenDoor));
        stringstream ss; ss << "Le Npc " << getNpcId() << " a ouvert la porte " << porte.getId() << " !";
        LOG(ss.str());
    }
}