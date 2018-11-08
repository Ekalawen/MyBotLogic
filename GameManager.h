#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Carte.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include "TurnInfo.h"

#include "MyBotLogic/Tools/Profiler.h"

#include "BehaviorTree/Composite/Selecteur.h"

#include <map>

using std::map;
using std::vector;
using std::string;

class npc_inexistant {};
class npc_deja_existant {};

class GameManager {
    static Logger logger, loggerRelease;
    map<int, Npc> npcs; // Les npcs sont stock�s par leurs ids
public:
    Carte carte;
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour d�terminer la strat�gie � suivre
    vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assign�s � des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(vector<Action*>& actionList) noexcept; // Remplie l'action liste !
    void reaffecterObjectifsSelonDistance(); // R�affecte les objectifs des Npcs entre
    void ordonnerMouvements(vector<Mouvement>& mouvements) noexcept; // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)
    void updateModel(const TurnInfo&) noexcept; // Met � jour le mod�le avec les informations que d�couvrent les NPCS
    void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { 
       Profiler profiler{ GameManager::getLogger(), "execute" }; 
       behaviorTreeManager.execute(); 
    };

    Npc& getNpcById(int id);
    map<int, Npc>& getNpcs();
    void addNpc(Npc npc);

    static void log(string str) noexcept { // Permet de d�bugger ! :D
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Log(str);
        #endif
    }
    static void logRelease(string _str) noexcept { // Permet de d�bugger ! :D
        loggerRelease.Log(_str);
    }
    static void setLog(string path, string fileName) noexcept { // Permet d'initialiser le logger =)
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Init(path, fileName);
        #endif
    }
    static void setLogRelease(string path, string fileName) noexcept { // Permet d'initialiser le logger =)
        loggerRelease.Init(path, fileName);
    }
    static Logger& getLogger() noexcept { // Permet d'initialiser le logger =)
#ifndef _DEBUG
       return;
#endif
#ifdef _DEBUG
       return logger;
#endif
    }
    static Logger& getLoggerRelease() noexcept { // Permet d'initialiser le logger =)
#ifndef _DEBUG
       return;
#endif
#ifdef _DEBUG
       return loggerRelease;
#endif
    }

private:
    void addNewTiles(const TurnInfo& ti) noexcept;
    void addNewObjects(const TurnInfo& ti) noexcept;
    vector<Mouvement> getAllMouvements();
    int getIndiceMouvementPrioritaire(vector<Mouvement>& mouvements, const vector<int>& indicesAConsiderer);
    void gererCollisionsMemeCaseCible(vector<Mouvement>& mouvements);
    void stopNonPrioritaireMouvements(vector<Mouvement>& mouvements, const vector<int>& indicesMouvementsSurMemeCaseCible, const int indiceMouvementPrioritaire, bool& continuer);
    void refreshFloodfill();
};

#endif