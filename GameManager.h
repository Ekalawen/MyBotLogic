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
    map<int, Npc> npcs; // Les npcs sont stockés par leurs ids

#ifdef BOT_LOGIC_DEBUG
#define GAME_MANAGER_LOG_DEBUG(text, autoEndLine) logger.Log(text, autoEndLine)
#define GAME_MANAGER_LOG_DEBUG(text) logger.Log(text)
#else
#define GAME_MANAGER_LOG_DEBUG(text, autoEndLine) 0
#define GAME_MANAGER_LOG_DEBUG(text) 0
#endif

#ifndef BOT_LOGIC_DEBUG
#define GAME_MANAGER_LOG_RELEASE(text, autoEndLine) loggerRelease.Log(text, autoEndLine)
#define GAME_MANAGER_LOG_RELEASE(text) loggerRelease.Log(text)
#else
#define GAME_MANAGER_LOG_RELEASE(text, autoEndLine) 0
#define GAME_MANAGER_LOG_RELEASE(text) 0
#endif

public:
    Carte c;
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour déterminer la stratégie à suivre
    vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assignés à des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(vector<Action*>& actionList) noexcept; // Remplie l'action liste !
    void reaffecterObjectifsSelonDistance(); // Réaffecte les objectifs des Npcs entre
    void ordonnerMouvements(vector<Mouvement>& mouvements) noexcept; // Permet d'ordonner les mouvements pour éviter les collisions et gérer les politesses de priorités =)
    void updateModel(const TurnInfo&) noexcept; // Met à jour le modèle avec les informations que découvrent les NPCS
    void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { 
       ProfilerDebug profiler{ GameManager::getLogger(), "execute" }; 
       ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "execute" };
       behaviorTreeManager.execute(); 
    };

    Npc& getNpcById(int id);
    map<int, Npc>& getNpcs();
    void addNpc(Npc npc);

    //static void log(string str) noexcept { // Permet de débugger ! :D
    //    #ifndef _DEBUG
    //        return;
    //    #endif
    //    #ifdef _DEBUG
    //        logger.Log(str);
    //    #endif
    //}
    //static void logRelease(string _str) noexcept { // Permet de débugger ! :D
    //    loggerRelease.Log(_str);
    //}

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
       return logger;
    }
    static Logger& getLoggerRelease() noexcept { // Permet d'initialiser le logger =)
       return loggerRelease;
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