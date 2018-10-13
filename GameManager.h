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
    static Logger logger;
public:
    Map m;
    map<int, Npc> npcs; // Les npcs sont stock�s par leurs ids
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour d�terminer la strat�gie � suivre
    vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assign�s � des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void associateNpcsWithObjectiv();
    void moveNpcs(vector<Action*>& actionList); // Remplie l'action liste !
    void ordonnerMouvements(vector<Mouvement*>& mouvements); // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)
    void updateModel(TurnInfo); // Met � jour le mod�le avec les informations que d�couvrent les NPCS
    void execute() { behaviorTreeManager.execute(); };

    static void Log(string str) { // Permet de d�bugger ! :D
        logger.Log(str);
    }
    static void SetLog(string path, string fileName) { // Permet d'initialiser le logger =)
        logger.Init(path, fileName);
    }

private:
    void addNewTiles(TurnInfo ti);
    void addNewObjects(TurnInfo ti);
};

#endif