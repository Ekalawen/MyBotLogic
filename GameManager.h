#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Carte.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include "TurnInfo.h"

#include "BehaviorTree/Composite/Selecteur.h"

#include <map>

class npc_inexistant {};
class npc_deja_existant {};

class GameManager {
    static Logger logger, loggerRelease;
    std::map<int, Npc> npcs; // Les npcs sont stock�s par leurs ids
public:
    Carte map;
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour d�terminer la strat�gie � suivre
    std::vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assign�s � des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(std::vector<Action*>& _actionList) noexcept; // Remplie l'action liste !
    void reaffecterObjectifsSelonDistance(); // R�affecte les objectifs des Npcs entre
    void ordonnerMouvements(std::vector<Mouvement>& _mouvements) noexcept; // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)
    void updateModel(const TurnInfo&) noexcept; // Met � jour le mod�le avec les informations que d�couvrent les NPCS
    void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { behaviorTreeManager.execute(); };

    Npc& getNpcById(int _id);
    std::map<int, Npc>& getNpcs();
    void addNpc(Npc _npc);

    static void log(std::string _str) noexcept { // Permet de d�bugger ! :D
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Log(_str);
        #endif
    }
    static void LogRelease(std::string _str) noexcept { // Permet de d�bugger ! :D
        loggerRelease.Log(_str);
    }
    static void setLog(std::string _path, std::string _fileName) noexcept { // Permet d'initialiser le logger =)
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Init(_path, _fileName);
        #endif
    }
    static void setLogRelease(std::string _path, std::string _fileName) noexcept { // Permet d'initialiser le logger =)
        loggerRelease.Init(_path, _fileName);
    }

private:
    void addNewTiles(const TurnInfo& _tile) noexcept;
    void addNewObjects(const TurnInfo& _tile) noexcept;
    std::vector<Mouvement> getAllMouvements();
    int getIndiceMouvementPrioritaire(std::vector<Mouvement>& _mouvements, const std::vector<int>& _indicesAConsiderer);
    void gererCollisionsMemeCaseCible(std::vector<Mouvement>& _mouvements);
    void stopNonPrioritaireMouvements(std::vector<Mouvement>& _mouvements, const std::vector<int>& _indicesMouvementsSurMemeCaseCible, const int _indiceMouvementPrioritaire, bool& _continuer);
};

#endif