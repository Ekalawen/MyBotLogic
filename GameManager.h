#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Map.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include "TurnInfo.h"

#include "BehaviorTree/Composite/Selecteur.h"

#include <map>

class GameManager {
    static Logger logger, loggerRelease;
public:
    Map m;
    map<int, Npc> npcs; // Les npcs sont stock�s par leurs ids
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour d�terminer la strat�gie � suivre
    vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assign�s � des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(vector<Action*>& actionList) noexcept; // Remplie l'action liste !
    void ordonnerMouvements(vector<Mouvement*>& mouvements) noexcept; // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)
    void updateModel(const TurnInfo&) noexcept; // Met � jour le mod�le avec les informations que d�couvrent les NPCS
    void GameManager::InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { behaviorTreeManager.execute(); };

    static void Log(string str) noexcept { // Permet de d�bugger ! :D
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Log(str);
        #endif
    }
    static void LogRelease(string str) noexcept { // Permet de d�bugger ! :D
        loggerRelease.Log(str);
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
    void addNewTiles(TurnInfo ti) noexcept;
    void addNewObjects(TurnInfo ti) noexcept;
};

#endif