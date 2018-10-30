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
    std::map<int, Npc> npcs; // Les npcs sont stockés par leurs ids
public:
    Carte map;
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour déterminer la stratégie à suivre
    std::vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assignés à des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(std::vector<Action*>& _actionList) noexcept; // Remplie l'action liste !
    void reaffecterObjectifsSelonDistance(); // Réaffecte les objectifs des Npcs entre
    void ordonnerMouvements(std::vector<Mouvement>& _mouvements) noexcept; // Permet d'ordonner les mouvements pour éviter les collisions et gérer les politesses de priorités =)
    void updateModel(const TurnInfo&) noexcept; // Met à jour le modèle avec les informations que découvrent les NPCS
    void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { behaviorTreeManager.execute(); };

    Npc& getNpcById(int _id);
    std::map<int, Npc>& getNpcs();
    void addNpc(Npc _npc);

    static void log(std::string _str) noexcept { // Permet de débugger ! :D
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Log(_str);
        #endif
    }
    static void LogRelease(std::string _str) noexcept { // Permet de débugger ! :D
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
    void addNewTiles(TurnInfo _tile) noexcept;
    void addNewObjects(TurnInfo _tile) noexcept;
    std::vector<Mouvement> getAllMouvements();
    int getIndiceMouvementPrioritaire(std::vector<Mouvement>& _mouvements, std::vector<int> _indicesAConsiderer);
    void gererCollisionsMemeCaseCible(std::vector<Mouvement>& _mouvements);
    void stopNonPrioritaireMouvements(std::vector<Mouvement>& _mouvements, std::vector<int> _indicesMouvementsSurMemeCaseCible, int _indiceMouvementPrioritaire, bool& _continuer);
};

#endif