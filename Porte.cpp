#include "MyBotLogic/Porte.h"
#include "ObjectInfo.h"
#include "Carte.h"
#include "GameManager.h"
#include <algorithm>

using std::set;

Porte::Porte(const ObjectInfo& porte, const Carte& c) 
    : id( porte.objectID )
{
    // On vérifie que notre objet est bien une porte
    if (find(porte.objectTypes.begin(), porte.objectTypes.end(), Object::ObjectType_Door) == porte.objectTypes.end())
        throw not_a_door{};

    // On set les attributs de base
    //id = porte.objectID;
    int voisin1 = porte.tileID;
	int voisin2 = c.getAdjacentTileAt(porte.tileID, porte.position);
    tilesVoisines.push_back(voisin1);
    if(voisin2 != -1)
        tilesVoisines.push_back(voisin2);
    if (find(porte.objectStates.begin(), porte.objectStates.end(), Object::ObjectState_Opened) != porte.objectStates.end())
        etat = Object::ObjectState_Opened;
    else
        etat = Object::ObjectState_Closed;
    //switchsIds = static_cast<std::set<int>>(porte.connectedTo);
    //std::transform(porte.connectedTo.begin(), porte.connectedTo.end(), switchsIds.begin(), [](unsigned int a) -> int {return static_cast<int>(a); });
    for (auto switchId : porte.connectedTo)
        switchsIds.insert(static_cast<int>(switchId));

    if (porte.connectedTo.size() == 0)
        type = A_POIGNEE;
    else
        type = A_SWITCH;
}

int Porte::getId() const noexcept {
    return  id;
}
Porte::Type Porte::getType() const noexcept {
    return type;
}
Object::EObjectState Porte::getEtat() const noexcept {
    return etat;
}
vector<int> Porte::getTilesVoisines() const noexcept {
    return tilesVoisines;
}
bool Porte::isVoisine(const int tileId) const noexcept {
    return find(tilesVoisines.begin(), tilesVoisines.end(), tileId) != tilesVoisines.end();
}
std::set<int> Porte::getSwitchsIds() const noexcept {
    return switchsIds;
}

std::set<int> Porte::getSwitchsTilesIds(const Carte& c) const noexcept {
    set<int> tilesIds;
    for (auto activateur : c.getActivateurs()) {
        if (find(switchsIds.begin(), switchsIds.end(), activateur.second.getId()) != switchsIds.end()) {
            tilesIds.insert(activateur.second.getTileId());
        }
    }
    return tilesIds;
}

void Porte::ouvrirPorte() {
    etat = Object::ObjectState_Opened;
}

void Porte::fermerPorte() {
    etat = Object::ObjectState_Closed;
}

bool Porte::canPassDoor(const int npcActif, const int caseAvantPorte, GameManager& gm, int& tempsAvantOuverture, vector<Contrainte>& contraintesDejaNecessaires) const noexcept {
    // Si la porte est ouverte et qu'elle n'est pas une porte à switch !
    if (getEtat() == Object::ObjectState_Opened && getType() != Porte::A_SWITCH) {
        tempsAvantOuverture = 0;
        //contraintesDejaNecessaires = {};
        return true;
    }

    // Si c'est une porte à poignée
    if (getType() == Porte::A_POIGNEE) {
        tempsAvantOuverture = 1;
        //contraintesDejaNecessaires = {};
        return true;
    }

    // Si c'est une porte à switch dont le switch est devant nous
    set<int> switchsTilesIds = getSwitchsTilesIds(gm.c);
    if (find(switchsTilesIds.begin(), switchsTilesIds.end(), caseAvantPorte) != switchsTilesIds.end()) {
        tempsAvantOuverture = 0;
        //contraintesDejaNecessaires = {};
        return true;
    }

    // TODO : il faut prendre la meilleure permutations de contraintes ici
    // A définir ce qu'est le "meilleur" !

    // Si c'est une porte à switch dont le switch n'est pas trivial, c'est là que les ennuis commencent !
    // On regarde si un autre Npc pourrait aller sur un switch
    vector<Contrainte> contraintesFinales{};
    bool auMoins1SolutionTrouvee = false;
    for (auto& npc_pair : gm.getNpcs()) {
        Npc& npcSuppleant = npc_pair.second;

        // On vérifie que le Npc n'est pas déjà occupé
        if (npcSuppleant.getId() != npcActif) {

            // On vérifie qu'il peut atteindre au moins l'un des interupteurs
            for (int switchTileId : switchsTilesIds) {

                // On crée notre contrainte
                vector<Contrainte> newContraintesDejaNecessaires = contraintesDejaNecessaires;
                int caseApresPorte = *find_if(tilesVoisines.begin(), tilesVoisines.end(), [caseAvantPorte](int tile) { return tile != caseAvantPorte; }); // Si ça plante ici, c'est que cette porte n'a qu'un seul voisin, donc c'est qu'on a essayé de passer à travers une porte sur le bord du terrain ! Pas bon !
                Contrainte contrainteActuelle{caseAvantPorte, caseApresPorte, switchsTilesIds, gm.c};
                contrainteActuelle.setNpc(npcSuppleant.getId());

                // On vérifie que cette contrainte n'existe pas déjà, sinon on renvoi false !
                auto it = find_if(newContraintesDejaNecessaires.begin(), newContraintesDejaNecessaires.end(), [caseAvantPorte, caseApresPorte](Contrainte& contrainte) {
                    return contrainte.getCaseAvantPorte() == caseAvantPorte && contrainte.getCaseApresPorte() == caseApresPorte;
                });
                if (it == newContraintesDejaNecessaires.end()) {
                    // On ajoute notre contrainte
                    newContraintesDejaNecessaires.push_back(contrainteActuelle);

                    // On essaye d'aller au switch
                    Chemin chemin = gm.c.aStar(npcSuppleant.getTileId(), switchTileId, npcSuppleant.getId(), gm, newContraintesDejaNecessaires);

                    if (chemin.isAccessible()) {
                        // On résout notre contrainte on on garde l'ensemble des nouvelles contraintes
                        newContraintesDejaNecessaires.back().resoudre(npcSuppleant.getId(), chemin);
                        if (contraintesFinales.empty() || Contrainte::prixContraintes(newContraintesDejaNecessaires) < Contrainte::prixContraintes(contraintesFinales)) {
                            contraintesFinales = newContraintesDejaNecessaires;
                            auMoins1SolutionTrouvee = true;
                        }
                        //return true;
                    }
                //} else {
                //    return false;
                }
            }
        }
    }
    if (auMoins1SolutionTrouvee) {
        contraintesDejaNecessaires = contraintesFinales;
        tempsAvantOuverture = contraintesDejaNecessaires.back().getCheminAssocie()->distance();
        return true;
    } else {
        return false;
    }
}
