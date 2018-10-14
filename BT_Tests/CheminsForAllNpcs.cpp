#include "CheminsForAllNpcs.h"
#include "../BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT CheminsForAllNpcs::execute() {
    GameManager::Log("CheminsForAllNpcs");
    // Précondition vérifié : il y a au moins autant d'objectifs que de npcs
    // Calculer si il existe un chemin pour un objectif unique pour chacun des npcs

    // Si on a déjà réussi cette méthode 1 fois, alors ça suffit et on a plus besoin de l'appeler à toutes les frames ! =)
    if (aDejaReussi) return ETAT_ELEMENT::REUSSI;

    // Pour cela on va calculer tous les chemins allant de tous les npcs à tous les objectifs
    // Et on va stocker cet ensemble de chemins, pour chaque npc, dans sa liste de cheminsPossibles
    for (auto& pair : gm.npcs) {
        // On récupère le npc
        Npc& npc = pair.second;
        npc.resetChemins();

        // On récupère tous les chemins à tous ses objectifs
        for (auto objectif : gm.m.objectifs) {
            Chemin chemin = gm.m.aStar(npc.tileId, objectif.second.id);
            npc.addChemin(chemin);
        }
    }

    // Ensuite on réinitialise la liste d'objectifsPris du GameManager
    gm.objectifPris = vector<int>{};

    // Puis en commancant par le Npc dont la plus courte distance à un objectif est la plus longue
    // On tente de lui affecte cet objectif et de le rajouter cet objectif dans objectifsPris
    // Tant qu'il reste des Npcs qui n'ont pas de chemins
    vector<int> npcAffectes;
    while (npcAffectes.size() < gm.npcs.size()) {
        // On récupère le npc qui a le chemin minimal le plus long
        Npc* lastNpc;
        int distMax = -1;
        Chemin cheminMin;
        for (auto& pair_npc : gm.npcs) {
            Npc& npc = pair_npc.second;
            if (find(npcAffectes.begin(), npcAffectes.end(), npc.id) == npcAffectes.end()) { // Si on a pas déjà affecté cet npc !
                Chemin chemin = npc.getCheminMinNonPris(gm.objectifPris, gm.m.tailleCheminMax());
                if (chemin.distance() > distMax) {
                    lastNpc = &npc;
                    distMax = chemin.distance();
                    cheminMin = chemin;
                }
            }
        }

        // Puis on lui affecte son chemin le plus court si il y en a un !
        if (cheminMin.distance() <= 0) {
            // Si le cheminMin n'a pas été initialisé, c'est qu'il n'y a pas de chemins pour tous les npcs !
            GameManager::Log("Il n'y a pas de chemins pour tous les npcs !");
            return ETAT_ELEMENT::ECHEC;
        }
        lastNpc->chemin = cheminMin;
        lastNpc->tileObjectif = cheminMin.destination();
        gm.objectifPris.push_back(cheminMin.destination());

        npcAffectes.push_back(lastNpc->id);
    }

    // Si on a réussi à affecter un objectif à tous les Npcs, alors on renvoie un succès
    aDejaReussi = true;
    return ETAT_ELEMENT::REUSSI;
}
