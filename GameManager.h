#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Carte.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include "LevelInfo.h"
#include "Globals.h"
#include "BehaviorTree/Composite/Selecteur.h"
#include <map>

using std::map;
using std::vector;
using std::string;

class npc_inexistant {};
class npc_deja_existant {};

struct TurnInfo;
class GameManager {
    static Logger logger, loggerRelease;
    map<int, Npc> npcs; // Les npcs sont stockés par leurs ids
public:
    Carte c; // La carte, et on utilise c pour gagner du temps !
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour déterminer la stratégie à suivre
    vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assignés à des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(vector<Action*>& actionList) noexcept; // Remplie l'action liste !
    void reafecterObjectifsSelonDistance(); // Réaffecte les objectifs des Npcs entre
    void ordonnerMouvements(vector<Mouvement>& mouvements) noexcept; // Permet d'ordonner les mouvements pour éviter les collisions et gérer les politesses de priorités =)
    void ordonnerMouvementsSelonSwitchs(vector<Mouvement>& _mouvements); // Permet de prioritariser les mouvements en rapports avec des switchs !
    void updateModel(const TurnInfo&) noexcept; // Met à jour le modèle avec les informations que découvrent les NPCS
    void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { behaviorTreeManager.execute(); };
    void affecterContraintes() noexcept; // Le but de cette fonction est de prendre en compte les contraintes pour que tous les Npcs puissent aller, à un moment donnée, à l'endroit où ils veulent aller
    void cleanContraintes() noexcept; // Permet de supprimer les contraintes non résolues

    Npc& getNpcById(int id);
    int getNpcIdAtTileId(int tileId);
    map<int, Npc>& getNpcs();
    void addNpc(Npc npc);

    static void log(string str) noexcept { // Permet de débugger ! :D
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Log(str);
        #endif
    }
    static void logRelease(string _str) noexcept { // Permet de débugger ! :D
        loggerRelease.Log(_str);
    }
    static void SetLog(string path, string fileName) noexcept { // Permet d'initialiser le logger =)
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Init(path, fileName);
        #endif
    }
    static void SetLogRelease(string path, string fileName) noexcept { // Permet d'initialiser le logger =)
        loggerRelease.Init(path, fileName);
    }

private:
    void addNewTiles(const TurnInfo& ti) noexcept;
    void addNewObjects(const TurnInfo& ti) noexcept;
    void majPortesASwitch() noexcept;
    vector<Mouvement> getAllMouvements();
    int getIndiceMouvementPrioritaire(vector<Mouvement>& mouvements, const vector<int>& indicesAConsiderer);
    void gererCollisionsMemeCaseCible(vector<Mouvement>& mouvements);
    void stopNonPrioritaireMouvements(vector<Mouvement>& mouvements, const vector<int>& indicesMouvementsSurMemeCaseCible, const int indiceMouvementPrioritaire, bool& continuer);
};

#endif