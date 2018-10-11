#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Map.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include <map>

class GameManager {
    static Logger logger;
public:
    Map m;
    map<int, Npc> npcs; // Les npcs sont stockés par leurs ids

    GameManager() = default;
    GameManager(LevelInfo);
    void associateNpcsWithObjectiv();
    void moveNpcs(vector<Action*>& actionList); // Remplie l'action liste !
    void ordonnerMouvements(vector<Mouvement*>& mouvements); // Permet d'ordonner les mouvements pour éviter les collisions et gérer les politesses de priorités =)

    static void Log(string str) { // Permet de débugger ! :D
        logger.Log(str);
    }
    static void SetLog(string path, string fileName) { // Permet d'initialiser le logger =)
        logger.Init(path, fileName);
    }
};

#endif