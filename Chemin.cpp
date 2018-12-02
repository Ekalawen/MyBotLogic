#include "Chemin.h"
#include "GameManager.h"
#include "Npc.h"

#include <vector>
#include <string>
#include <algorithm>

using std::to_string;

Chemin::Chemin()
    : chemin{ vector<int>{} },
    inaccessible{ false },
    contraintes{ vector<Contrainte>{} }
{
}

int Chemin::distance() const noexcept {
    if (!inaccessible) {
        if (contraintes.empty()) {
            return static_cast<int>(chemin.size());
        } else {
            // Si on a des contraintes, il faut voir laquelle nous ralentit le plus et rajouter ce score à la taille du chemin !
            int ralentissementMax = 0;
            for (Contrainte contrainte : contraintes) {
                if (contrainte.estResolue()) {
                    int distanceCaseApresContrainte = std::distance(chemin.begin(), find(chemin.begin(), chemin.end(), contrainte.getCaseApresPorte())); // Je n'ai pas confiance en cette ligne !
                    int tempsAvantContrainteResolu = contrainte.getCheminAssocie()->distance();
                    int ralentissement = tempsAvantContrainteResolu - distanceCaseApresContrainte; // On pourrait rajouter -1 pour simuler le fait que l'on peut ordonner les mouvements, mais je vais éviter
                    if (ralentissement > ralentissementMax) {
                        ralentissementMax = ralentissement;
                    }
                } else {
                    //LOG("Appel de distance sur un chemin avec des contraintes non-résolues !");
                }
            }
            return static_cast<int>(chemin.size()) + ralentissementMax;
        }
    } else {
        LOG("Distance appelée sur un chemin inacessible !");
        exit(0);
        //return -1;
    }
}

int Chemin::distanceBrute() const noexcept {
    if (!inaccessible) {
            return static_cast<int>(chemin.size());
    } else {
        LOG("Distance appelée sur un chemin inacessible !");
        exit(0);
        //return -1;
    }
}

int Chemin::destination() const noexcept {
   if (!chemin.empty() && !inaccessible) {
      return chemin[0];
   }
   else {
      LOG("Attention, on essaye de récupérer la destination d'un chemin vide !");
      return -1;
   }
}

void Chemin::setInaccessible() noexcept {
   inaccessible = true;
}

bool Chemin::isAccessible() const noexcept {
   return !inaccessible;
}

bool Chemin::empty() const noexcept {
   return chemin.empty();
}

string Chemin::toString() const noexcept {
   string str;
   if (chemin.size() <= 0) {
      str += "vide";
   }
   else {
      if (!isAccessible()) {
         str += "inaccessible : ";
      }
      for (int i = static_cast<int>(chemin.size() - 1); i >= 0; i--) {
         str += to_string(chemin[i]) + " ";
      }
      str.pop_back();
   }
   return str;
}


void Chemin::removeFirst() {
   if (empty())
      throw chemin_vide{};
   else
      chemin.pop_back();
}

void Chemin::addFirst(const int elem) {
   chemin.push_back(elem);
}

int Chemin::getFirst() const {
   if (empty())
      throw chemin_vide{};
   else
      return chemin.back();
}

void Chemin::resetChemin() noexcept {
   chemin.clear();
   inaccessible = false;
}

void Chemin::cleanContraintes() {
    bool continuer = true;
    while (continuer) {
        auto it = find_if(contraintes.begin(), contraintes.end(), [](Contrainte contrainte) {
            return !contrainte.estResolue();
        });
        if (it != contraintes.end()) {
            contraintes.erase(it);
        } else {
            continuer = false;
        }
    }
}

void Chemin::addContrainte(const Contrainte& contrainte) {
    contraintes.push_back(contrainte);
}
void Chemin::setContraintes(const vector<Contrainte> nouvellesContraintes) {
    contraintes = nouvellesContraintes;
}
void Chemin::resetContraintes() {
    contraintes.clear();
}
vector<Contrainte>& Chemin::getContraintes() {
    return contraintes;
}

vector<int> Chemin::affecterContraintes(int npcAffecte, GameManager& gm) {
    vector<int> npcNecessaires{ npcAffecte }; // Les npcs qui ont étés utilisées pour satisfaire cette contrainte !

    // Si on a au moins 1 contrainte ...
    if (!getContraintes().empty()) {
        bool estApplicable = true;
        // TODO : s'assurer que l'on prenne la première contrainte sur le chemin !
        Contrainte& contrainte = getContraintes().back();

        // On vérifie que la contrainte est déjà résolue !
        if (!contrainte.estResolue()) {
            estApplicable = false;
            LOG("Erreur ! Des contraintes n'ont pas étés résolues !");
        }

        // On vérifie que le npc est dispo
        if (contrainte.getNpcAssocie() == npcAffecte) {
            estApplicable = false;
            LOG("Erreur ! J'ai fais n'imp !");
        }

        // Si c'est bon alors on applique la première contrainte
        if (estApplicable) {
            LOG("Contrainte affectée au Npc " + to_string(contrainte.getNpcAssocie()) + " par le Npc " + to_string(npcAffecte) + " où il faut aller en " + to_string(contrainte.getCheminAssocie()->destination()));
            gm.getNpcById(contrainte.getNpcAssocie()).setChemin(*contrainte.getCheminAssocie());
            npcNecessaires.push_back(contrainte.getNpcAssocie());

            // Puis on appelle cette fonction sur le chemin de notre npcNecessaire !
            vector<int> toAdd = contrainte.getCheminAssocie()->affecterContraintes(contrainte.getNpcAssocie(), gm);
            npcNecessaires.insert(npcNecessaires.end(), toAdd.begin(), toAdd.end());
        }
    }

    return npcNecessaires;


    //// On vérifie d'abord que toutes les contraintes sont applicables, cad que tous les npcs pour les faires sont disponibles
    //bool sontApplicables = true;
    //for (Contrainte& contrainte : getContraintes()) {
    //    // On vérifie que la contrainte est déjà résolue !
    //    if (!contrainte.estResolue()) {
    //        sontApplicables = false;
    //        LOG("Erreur ! Des contraintes n'ont pas étés résolues !");
    //        break;
    //    }

    //    // On vérifie que le npc est dispo
    //    if (find_if(npcsDisponibles.begin(), npcsDisponibles.end(), [&contrainte](Npc* npc) {
    //        return contrainte.getNpcAssocie() == npc->getId(); }) == npcsDisponibles.end())
    //    {
    //        sontApplicables = false;
    //        break;
    //    }
    //}

    //// Si c'est bon, alors on les appliques !
    //if (sontApplicables) {
    //    for (Contrainte& contrainte : getContraintes()) {
    //        auto it = find_if(npcsDisponibles.begin(), npcsDisponibles.end(), [&contrainte](Npc* npc) {
    //            return npc->getId() == contrainte.getNpcAssocie(); });
    //        (*it)->setChemin(*contrainte.getCheminAssocie());
    //        npcsDisponibles.erase(it);
    //    }
    //}

    //// Puis on appelle cette fonction sur tous les chemins de nos contraintes ! :3
    //for (Contrainte& contrainte : getContraintes()) {
    //    contrainte.getCheminAssocie()->affecterContraintes(npcsDisponibles);
    //}
}
