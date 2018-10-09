#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Map.h"
#include "Npc.h"
#include "Logger.h"
#include <map>

class GameManager {
    static Logger logger;
public:
    Map m;
    map<int, Npc> npcs; // Les npcs sont stock�s par leurs ids

    GameManager() = default;
    GameManager(LevelInfo);
    void associateNpcsWithObjectiv();
    void moveNpcs(vector<Action*>& actionList); // Remplie l'action liste !
    void ordonnerMouvements(vector<tuple<int, Tile::ETilePosition>>& mouvements); // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)

    static void Log(string str) { // Permet de d�bugger ! :D
        logger.Log(str);
    }
    static void SetLog(string path, string fileName) { // Permet d'initialiser le logger =)
        logger.Init(path, fileName);
    }
};

#endif